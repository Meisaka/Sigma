#include "systems/OpenGLSystem.h"
#include "systems/GLSLShader.h"
#include "systems/GLSixDOFView.h"
#include "controllers/FPSCamera.h"
#include "controllers/RiftCamera.h"
#include "components/GLSprite.h"
#include "components/GLIcoSphere.h"
#include "components/GLCubeSphere.h"
#include "components/GLMesh.h"
#include "components/GLScreenQuad.h"
#include "components/PointLight.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

namespace Sigma{
	// RenderTarget methods
	RenderTarget::~RenderTarget() {
		glDeleteTextures(1, &this->texture_id); // Perhaps should check if texture was created for this RT or is used elsewhere
		glDeleteRenderbuffers(1, &this->depth_id);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &this->fbo_id);
	}

	void RenderTarget::Use(int slot) {
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo_id);
	}

	std::map<std::string, Sigma::resource::GLTexture> OpenGLSystem::textures;
    OpenGLSystem::OpenGLSystem() : windowWidth(1024), windowHeight(768), deltaAccumulator(0.0),
		framerate(60.0f), viewMode(""), renderMode(GLS_NONE) {}

	std::map<std::string, Sigma::IFactory::FactoryFunction>
        OpenGLSystem::getFactoryFunctions() {
		using namespace std::placeholders;

		std::map<std::string, Sigma::IFactory::FactoryFunction> retval;
		retval["GLSprite"] = std::bind(&OpenGLSystem::createGLSprite,this,_1,_2);
		retval["GLIcoSphere"] = std::bind(&OpenGLSystem::createGLIcoSphere,this,_1,_2);
		retval["GLCubeSphere"] = std::bind(&OpenGLSystem::createGLCubeSphere,this,_1,_2);
		retval["GLMesh"] = std::bind(&OpenGLSystem::createGLMesh,this,_1,_2);
		retval["FPSCamera"] = std::bind(&OpenGLSystem::createGLView,this,_1,_2, "FPSCamera");
		retval["RiftCamera"] = std::bind(&OpenGLSystem::createGLView,this,_1,_2, "RiftCamera");
		retval["GLSixDOFView"] = std::bind(&OpenGLSystem::createGLView,this,_1,_2, "GLSixDOFView");
		retval["PointLight"] = std::bind(&OpenGLSystem::createPointLight,this,_1,_2);
		retval["GLScreenQuad"] = std::bind(&OpenGLSystem::createScreenQuad,this,_1,_2);

        return retval;
    }

	IComponent* OpenGLSystem::createGLView(const unsigned int entityID, const std::vector<Property> &properties, std::string mode) {
		viewMode = mode;

		if(mode=="FPSCamera") {
			this->views.push_back(new Sigma::event::handler::FPSCamera(entityID));
		}
		else if(mode=="RiftCamera") {
			this->views.push_back(new Sigma::event::handler::RiftCamera(entityID));
		}
		else if(mode=="GLSixDOFView") {
			this->views.push_back(new GLSixDOFView(entityID));
		}
		else {
			std::cerr << "Invalid view type!" << std::endl;
			return nullptr;
		}

		float x=0.0f, y=0.0f, z=0.0f, rx=0.0f, ry=0.0f, rz=0.0f;

		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property*  p = &(*propitr);

			if (p->GetName() == "x") {
				x = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "y") {
				y = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "z") {
				z = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "rx") {
				rx = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "ry") {
				ry = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "rz") {
				rz = p->Get<float>();
				continue;
			}
		}

		this->views[this->views.size() - 1]->Transform.Move(x,y,z);
		this->views[this->views.size() - 1]->Transform.Rotate(rx,ry,rz);

		return this->views[this->views.size() - 1];
	}

	IComponent* OpenGLSystem::createGLSprite(const unsigned int entityID, const std::vector<Property> &properties) {
		GLSprite* spr = new GLSprite(entityID);
		float scale = 1.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		int componentID = 0;
		std::string textureFilename;

		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property*  p = &(*propitr);
			if (p->GetName() == "scale") {
				scale = p->Get<float>();
			}
			else if (p->GetName() == "x") {
				x = p->Get<float>();
			}
			else if (p->GetName() == "y") {
				y = p->Get<float>();
			}
			else if (p->GetName() == "z") {
				z = p->Get<float>();
			}
			else if (p->GetName() == "id") {
				componentID = p->Get<int>();
			}
			else if (p->GetName() == "textureFilename"){
				textureFilename = p->Get<std::string>();
			}
		}

		// Check if the texture is loaded and load it if not.
		if (textures.find(textureFilename) == textures.end()) {
			Sigma::resource::GLTexture texture;
			texture.LoadDataFromFile(textureFilename);
			if (texture.GetID() != 0) {
				Sigma::OpenGLSystem::textures[textureFilename] = texture;
			}
		}

		// It should be loaded, but in case an error occurred double check for it.
		if (textures.find(textureFilename) != textures.end()) {
			spr->SetTexture(&Sigma::OpenGLSystem::textures[textureFilename]);
		}
		spr->LoadShader();
		spr->Transform()->Scale(glm::vec3(scale));
		spr->Transform()->Translate(x,y,z);
		spr->InitializeBuffers();
		this->addComponent(entityID,spr);
		return spr;
	}

	IComponent* OpenGLSystem::createGLIcoSphere(const unsigned int entityID, const std::vector<Property> &properties) {
			Sigma::GLIcoSphere* sphere = new Sigma::GLIcoSphere(entityID);
			float scale = 1.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;

			int componentID = 0;
			std::string shader_name = "shaders/icosphere";

			for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
				const Property*  p = &(*propitr);
				if (p->GetName() == "scale") {
					scale = p->Get<float>();
					continue;
				}
				else if (p->GetName() == "x") {
					x = p->Get<float>();
					continue;
				}
				else if (p->GetName() == "y") {
					y = p->Get<float>();
					continue;
				}
				else if (p->GetName() == "z") {
					z = p->Get<float>();
					continue;
				}
				else if (p->GetName() == "id") {
					componentID = p->Get<int>();
				}
				else if (p->GetName() == "shader"){
					shader_name = p->Get<std::string>();
				}
				else if (p->GetName() == "lightEnabled") {
					sphere->SetLightingEnabled(p->Get<bool>());
				}
			}
			sphere->Transform()->Scale(scale,scale,scale);
			sphere->Transform()->Translate(x,y,z);
			sphere->LoadShader(shader_name);
			sphere->InitializeBuffers();
			sphere->SetCullFace("back");
			this->addComponent(entityID,sphere);
			return sphere;
	}

	IComponent* OpenGLSystem::createGLCubeSphere(const unsigned int entityID, const std::vector<Property> &properties) {
		Sigma::GLCubeSphere* sphere = new Sigma::GLCubeSphere(entityID);

		std::string texture_name = "";
		std::string shader_name = "shaders/cubesphere";
		std::string cull_face = "back";
		int subdivision_levels = 1;
		float rotation_speed = 0.0f;
		bool fix_to_camera = false;
		bool infinite_distance = false;

		float scale = 1.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float rx = 0.0f;
		float ry = 0.0f;
		float rz = 0.0f;
		int componentID = 0;
		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property*  p = &(*propitr);
			if (p->GetName() == "scale") {
				scale = p->Get<float>();
			}
			else if (p->GetName() == "x") {
				x = p->Get<float>();
			}
			else if (p->GetName() == "y") {
				y = p->Get<float>();
			}
			else if (p->GetName() == "z") {
				z = p->Get<float>();
			}
			else if (p->GetName() == "rx") {
				rx = p->Get<float>();
			}
			else if (p->GetName() == "ry") {
				ry = p->Get<float>();
			}
			else if (p->GetName() == "rz") {
				rz = p->Get<float>();
			}
			else if (p->GetName() == "subdivision_levels") {
				subdivision_levels = p->Get<int>();
			}
			else if (p->GetName() == "texture") {
				texture_name = p->Get<std::string>();
			}
			else if (p->GetName() == "shader") {
				shader_name = p->Get<std::string>();
			}
			else if (p->GetName() == "id") {
				componentID = p->Get<int>();
			}
			else if (p->GetName() == "cullface") {
				cull_face = p->Get<std::string>();
			}
			else if (p->GetName() == "fix_to_camera") {
				fix_to_camera = p->Get<bool>();
			}
			else if (p->GetName() == "infinite_distance") {
				infinite_distance = p->Get<bool>();
			}
			else if (p->GetName() == "lightEnabled") {
				sphere->SetLightingEnabled(p->Get<bool>());
			}
		}

		sphere->SetSubdivisions(subdivision_levels);
		sphere->SetFixToCamera(fix_to_camera);
		sphere->SetInfiniteDistance(infinite_distance);
		sphere->SetCullFace(cull_face);
		sphere->Transform()->Scale(scale,scale,scale);
		sphere->Transform()->Rotate(rx,ry,rz);
		sphere->Transform()->Translate(x,y,z);
		sphere->LoadShader(shader_name);
		sphere->LoadTexture(texture_name);
		sphere->InitializeBuffers();
		this->addComponent(entityID,sphere);
		return sphere;
	}

	IComponent* OpenGLSystem::createGLMesh(const unsigned int entityID, const std::vector<Property> &properties) {
		Sigma::GLMesh* mesh = new Sigma::GLMesh(entityID);

		float scale = 1.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float rx = 0.0f;
		float ry = 0.0f;
		float rz = 0.0f;
		int componentID = 0;
		std::string cull_face = "back";
		std::string shaderfile = "";

		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property*  p = &*propitr;
			if (p->GetName() == "scale") {
				scale = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "x") {
				x = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "y") {
				y = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "z") {
				z = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "rx") {
				rx = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "ry") {
				ry = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "rz") {
				rz = p->Get<float>();
				continue;
			}
			else if (p->GetName() == "meshFile") {
				std::cerr << "Loading mesh: " << p->Get<std::string>() << std::endl;
				mesh->LoadMesh(p->Get<std::string>());
			}
			else if (p->GetName() == "shader"){
				shaderfile = p->Get<std::string>();
			}
			else if (p->GetName() == "id") {
				componentID = p->Get<int>();
			}
			else if (p->GetName() == "cullface") {
				cull_face = p->Get<std::string>();
			}
			else if (p->GetName() == "lightEnabled") {
				mesh->SetLightingEnabled(p->Get<bool>());
			}
		}

		mesh->SetCullFace(cull_face);
		mesh->Transform()->Scale(scale,scale,scale);
		mesh->Transform()->Translate(x,y,z);
		mesh->Transform()->Rotate(rx,ry,rz);
		if(shaderfile != "") {
			mesh->LoadShader(shaderfile);
		}
		else {
			mesh->LoadShader(); // load default
		}
        mesh->InitializeBuffers();
        this->addComponent(entityID,mesh);
        return mesh;
    }

	IComponent* OpenGLSystem::createScreenQuad(const unsigned int entityID, const std::vector<Property> &properties) {
		Sigma::GLScreenQuad* quad = new Sigma::GLScreenQuad(entityID);

		float x = 0.0f;
		float y = 0.0f;
		float w = 0.0f;
		float h = 0.0f;
		int componentID = 0;
		std::string textrueName;
		bool textureInMemory = false;

		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property*  p = &(*propitr);
			if (p->GetName() == "left") {
				x = p->Get<float>();
			}
			else if (p->GetName() == "top") {
				y = p->Get<float>();
			}
			else if (p->GetName() == "width") {
				w = p->Get<float>();
			}
			else if (p->GetName() == "height") {
				h = p->Get<float>();
			}
			else if (p->GetName() == "textureName") {
				textrueName = p->Get<std::string>();
				textureInMemory = true;
			}
			else if (p->GetName() == "textureFileName") {
				textrueName = p->Get<std::string>();
			}
		}

		// Check if the texture is loaded and load it if not.
		if (textures.find(textrueName) == textures.end()) {
			Sigma::resource::GLTexture texture;
			if (textureInMemory) { // We are using an in memory texture. It will be populated somewhere else
				Sigma::OpenGLSystem::textures[textrueName] = texture;
			}
			else { // The texture in on disk so load it.
				texture.LoadDataFromFile(textrueName);
				if (texture.GetID() != 0) {
					Sigma::OpenGLSystem::textures[textrueName] = texture;
				}
			}
		}

		// It should be loaded, but in case an error occurred double check for it.
		if (textures.find(textrueName) != textures.end()) {
			quad->SetTexture(&Sigma::OpenGLSystem::textures[textrueName]);
		}

		quad->SetPosition(x, y);
		quad->SetSize(w, h);
		quad->LoadShader("shaders/quad");
		quad->InitializeBuffers();
		this->screensSpaceComp.push_back(std::unique_ptr<IGLComponent>(quad));
		return quad;
	}

	IComponent* OpenGLSystem::createPointLight(const unsigned int entityID, const std::vector<Property> &properties) {
		Sigma::PointLight *light = new Sigma::PointLight(entityID);

		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property*  p = &*propitr;
			if (p->GetName() == "x") {
				light->position.x = p->Get<float>();
			}
			else if (p->GetName() == "y") {
				light->position.y = p->Get<float>();
			}
			else if (p->GetName() == "z") {
				light->position.z = p->Get<float>();
			}
			else if (p->GetName() == "intensity") {
				light->intensity = p->Get<float>();
			}
			else if (p->GetName() == "cr") {
				light->color.r = p->Get<float>();
			}
			else if (p->GetName() == "cg") {
				light->color.g = p->Get<float>();
			}
			else if (p->GetName() == "cb") {
				light->color.b = p->Get<float>();
			}
			else if (p->GetName() == "ca") {
				light->color.a = p->Get<float>();
			}
			else if (p->GetName() == "radius") {
				light->radius = p->Get<float>();
			}
			else if (p->GetName() == "falloff") {
				light->falloff = p->Get<float>();
			}
		}

		this->addComponent(entityID, light);
		return light;
	}

	bool OpenGLSystem::SetStereoMode(const OVR::HMDInfo& riftinfo, GLSysRenderMode mode) {
		if(mode == GLS_RIFT) {
			const float scalefactor = 1.25f;
			GLsizei ossx,ossy;
			GLsizei riftresx,riftresy;
			float aspect;
			float sfov;
			float pofs;
			float pshift;
			float xcen,scf;
			float scrw, scrh, scrx;

			// Math for rift stereo, definitions are somewhat scattered through RiftSDK.
			riftresx = riftinfo.HResolution;
			ossx = (GLsizei)((float)riftresx * scalefactor);
			riftresy = riftinfo.VResolution;
			ossy = (GLsizei)((float)riftresy * scalefactor);
			scrw = 0.5f * (float)riftresx / (float)riftresx;
			scrh = (float)riftresy / (float)riftresy; // I know this is 1, but it *might* change
			scrx = ((float)riftresx * 0.5f) / (float)riftresx;
			sfov = 2.0f * atan(riftinfo.VScreenSize / (2.0f * riftinfo.EyeToScreenDistance));
			pshift = (riftinfo.HScreenSize * 0.25f) - (riftinfo.LensSeparationDistance * 0.5f);
			pofs = (4.0f * pshift) / riftinfo.HScreenSize;
			
			aspect = ((float)riftresx) / (2.0f * (float)riftresy);
			scf = 1.0f / scalefactor;
			xcen = 1.0f - (2.0f * riftinfo.LensSeparationDistance / riftinfo.HScreenSize);
			riftLensCenterL.x = (scrw + xcen * 0.5f) * 0.5f;
			riftLensCenterL.y = scrh * 0.5f;
			riftLensCenterR.x = scrx + (scrw - xcen * 0.5f) * 0.5f;
			riftLensCenterR.y = scrh * 0.5f;
			riftScaleOut.x = (scrw / 2.0f) * scf;
			riftScaleOut.y = (scrh / 2.0f) * scf * aspect;
			riftScaleIn.x = (2.0f / scrw);
			riftScaleIn.y = (2.0f / scrh) / aspect;
			riftScreenCenterL.x = scrw * 0.5f;
			riftScreenCenterL.y = scrh * 0.5f;
			riftScreenCenterR.x = scrx + (scrw * 0.5f);
			riftScreenCenterR.y = scrh * 0.5f;
			riftDistortionK = glm::vec4(
				riftinfo.DistortionK[0],
				riftinfo.DistortionK[1],
				riftinfo.DistortionK[2],
				riftinfo.DistortionK[3]
				);
			riftChromaK = glm::vec4(
				riftinfo.ChromaAbCorrection[0],
				riftinfo.ChromaAbCorrection[1],
				riftinfo.ChromaAbCorrection[2],
				riftinfo.ChromaAbCorrection[3]
				);
			stereoFBTw = ossx; // frame buffer size (not the same as screen size, usually larger)
			stereoFBTh = ossy;
			stereoLeftVPx = 0; // viewport settings (left)
			stereoLeftVPy = 0;
			stereoLeftVPw = ossx / 2;
			stereoLeftVPh = ossy;
			stereoRightVPx = stereoLeftVPw + stereoLeftVPx; // viewport settings (right)
			stereoRightVPy = 0;
			stereoRightVPw = ossx / 2;
			stereoRightVPh = ossy;
			// Generate projection matrix for each eye
			glm::mat4 projbase = glm::perspective(glm::degrees(sfov), aspect, 0.1f, 10000.0f);
			this->stereoProjectionLeft = glm::translate(pofs, 0.0f, 0.0f) * projbase;
			this->stereoProjectionRight = glm::translate(-pofs, 0.0f, 0.0f) * projbase;

			// translate view matrix X Coord by these
			stereoViewIPD = riftinfo.InterpupillaryDistance * 0.1f;
			stereoViewLeft = stereoViewIPD * 0.5f;
			stereoViewRight = stereoViewIPD * -0.5f;
			if(this->renderTargets.size() < 1) {
				this->createRenderTarget(ossx, ossy, GL_RGBA8);
			}
		}
		renderMode = mode;
		return true;
	}

	bool OpenGLSystem::SetStereoMode(GLSysRenderMode mode) {
		renderMode = mode;
		return true;
	}

	int OpenGLSystem::createRenderTarget(const unsigned int w, const unsigned int h, const unsigned int format) {
		std::unique_ptr<RenderTarget> newRT(new RenderTarget());

		glGenTextures(1, &newRT->texture_id);
		glBindTexture(GL_TEXTURE_2D, newRT->texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//NULL means reserve texture memory, but texels are undefined
		glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, (GLsizei)w, (GLsizei)h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glGenFramebuffers(1, &newRT->fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, newRT->fbo_id);
		
		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newRT->texture_id, 0);
		
		glGenRenderbuffers(1, &newRT->depth_id);
		glBindRenderbuffer(GL_RENDERBUFFER, newRT->depth_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
		
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, newRT->depth_id);
		
		//Does the GPU support current FBO configuration?
		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		switch(status) {
			case GL_FRAMEBUFFER_COMPLETE:
				std::cout << "Successfully created render target." << std::endl;
				break;
			default:
				std::cerr << "Error: Framebuffer format is not compatible." << std::endl;
		}

		// Unbind objects
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		this->renderTargets.push_back(std::move(newRT));
		return (this->renderTargets.size() - 1);
	}

    bool OpenGLSystem::Update(const double delta) {
        this->deltaAccumulator += delta;

        // Check if the deltaAccumulator is greater than 1/<framerate>th of a second.
        //  ..if so, it's time to render a new frame
        if (this->deltaAccumulator > 1000.0 / this->framerate) {
            
			// Hacky for now, but if we created at least one render target
			// then the 0th one is the draw buffer, 1+ could be for post-processing
			if(renderMode != GLS_NONE && this->renderTargets.size() > 0) {
				// Bind the primary render target
				glBindFramebuffer(GL_FRAMEBUFFER, this->renderTargets[0]->fbo_id);
			}

			//glm::vec3 viewPosition;
			glm::mat4 viewMatrix;
			glm::mat4 viewInfMatrix;
			glm::mat4 viewProj;
			glm::mat4 ProjMatrix;
			
			int looprender;
			int renderstage;
			if(renderMode == GLS_RIFT) { looprender = 1; } else { looprender = 0; }

            glClearColor(0.0f,0.0f,0.0f,0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers
			for(renderstage = 0; renderstage <= looprender; renderstage++) {
            // Set up the scene to a "clean" state.

			if(renderMode == GLS_RIFT && this->renderTargets.size() > 0) {
				if(renderstage == 0) { // Set the viewport size to half fill the framebuffer
				glViewport(stereoLeftVPx, stereoLeftVPy, stereoLeftVPw, stereoLeftVPh);
				}
				else {
				glViewport(stereoRightVPx, stereoRightVPy, stereoRightVPw, stereoRightVPh);
				}

				if (this->views.size() > 0) {
					viewInfMatrix = this->views[this->views.size() - 1]->GetViewMatrix(VIEW_INFINITE);
				}
				if(renderstage == 0) {
					if (this->views.size() > 0) {
						viewMatrix = this->views[this->views.size() - 1]->GetViewMatrix(VIEW_LEFT,stereoViewIPD);
						//viewPosition = this->views[this->views.size() - 1]->Transform.GetPosition();
					}
					//viewMatrix = glm::translate(viewMatrix,-stereoViewLeft,0.0f,0.0f);
					viewProj = viewMatrix;
					ProjMatrix = this->stereoProjectionLeft;
					viewProj = ProjMatrix * viewProj;
				}
				else {
					if (this->views.size() > 0) {
						viewMatrix = this->views[this->views.size() - 1]->GetViewMatrix(VIEW_RIGHT,stereoViewIPD);
						//viewPosition = this->views[this->views.size() - 1]->Transform.GetPosition();
					}
					//viewMatrix = glm::translate(viewMatrix,-stereoViewRight,0.0f,0.0f);
					viewProj = viewMatrix;
					ProjMatrix = this->stereoProjectionRight;
					viewProj = ProjMatrix * viewProj;
				}
			}
			else {
				if(this->renderTargets.size() > 0) {
					glViewport(0, 0, stereoFBTw, stereoFBTh); // Set the viewport size to fill the framebuffer
				}
				else {
					glViewport(0, 0, windowWidth, windowHeight); // Set the viewport size to fill the window
				}
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

				if (this->views.size() > 0) {
					viewInfMatrix = this->views[this->views.size() - 1]->GetViewMatrix();
					//viewPosition = this->views[this->views.size() - 1]->Transform.GetPosition();
				}
				viewProj = viewMatrix = viewInfMatrix;
				viewProj *= this->ProjectionMatrix;
				ProjMatrix= this->ProjectionMatrix;
			}
			// Loop through each light, rendering all components
			// TODO: Cull components based on light
			// TODO: Implement scissors test
			// Potentially move to deferred shading depending on
			// visual style needs
			
			// Ambient Pass
			// Loop through and draw each component.

			

			// Calculate frustum for culling
			this->GetView(0)->CalculateFrustum(viewProj);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			for (auto eitr = this->_Components.begin(); eitr != this->_Components.end(); ++eitr) {
				for (auto citr = eitr->second.begin(); citr != eitr->second.end(); ++citr) {
					IGLComponent *glComp = dynamic_cast<IGLComponent *>(citr->second.get());

					if(glComp) {
						glComp->GetShader()->Use();

						// Set view position
						//glUniform3f(glGetUniformBlockIndex(glComp->GetShader()->GetProgram(), "viewPosW"), viewPosition.x, viewPosition.y, viewPosition.z);

						// For now, turn on ambient intensity and turn off lighting
						glUniform1f(glGetUniformLocation(glComp->GetShader()->GetProgram(), "ambLightIntensity"), 0.05f);
						glUniform1f(glGetUniformLocation(glComp->GetShader()->GetProgram(), "diffuseLightIntensity"), 0.0f);
						glUniform1f(glGetUniformLocation(glComp->GetShader()->GetProgram(), "specularLightIntensity"), 0.0f);
						if(glComp->IsInfiniteDistance()) {
							glComp->Render(&viewInfMatrix[0][0], &ProjMatrix[0][0]);
						}
						else {
							glComp->Render(&viewMatrix[0][0], &ProjMatrix[0][0]);
						}
					}
				}
			}

			// Light passes
			for(auto eitr = this->_Components.begin(); eitr != this->_Components.end(); ++eitr) {
				for (auto citr = eitr->second.begin(); citr != eitr->second.end(); ++citr) {
					// Check if this component is a light
					PointLight *light = dynamic_cast<PointLight*>(citr->second.get());

					// If it is a light, and it intersects the frustum, then render
					if(light/* && this->GetView(0)->CameraFrustum.isectSphere(light->position, light->radius)*/) {
						// Modify depth test to allow for overlaying
						// lights
						glDepthFunc(GL_EQUAL);

						// Make sure additive blending is enabled
						glEnable(GL_BLEND);
						glBlendFunc(GL_ONE, GL_ONE);

						// Loop through and draw each component.
						for (auto child_eitr = this->_Components.begin(); child_eitr != this->_Components.end(); ++child_eitr) {
							for (auto child_citr = child_eitr->second.begin(); child_citr != child_eitr->second.end(); ++child_citr) {
								IGLComponent *glComp = dynamic_cast<IGLComponent *>(child_citr->second.get());

								if(glComp && glComp->IsLightingEnabled()) {
									glComp->GetShader()->Use();

									// Turn off ambient light for additive blending
									glUniform1f(glGetUniformLocation(glComp->GetShader()->GetProgram(), "ambLightIntensity"), 0.0f);

									// Set view position
									//glUniform3f(glGetUniformBlockIndex(glComp->GetShader()->GetProgram(), "viewPosW"), viewPosition.x, viewPosition.y, viewPosition.z);

									// Activate the current point light for this shader
									light->Activate(glComp->GetShader().get());

									// Render
									if(glComp->IsInfiniteDistance()) {
										glComp->Render(&viewInfMatrix[0][0], &ProjMatrix[0][0]);
									}
									else {
										glComp->Render(&viewMatrix[0][0], &ProjMatrix[0][0]);
									}
								}
							}
						}

						// Remove blending
						glDisable(GL_BLEND);

						// Re-enabled depth test
						glDepthFunc(GL_LESS);
					}
				}
			}

			// Enable transparent rendering
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			for (auto citr = this->screensSpaceComp.begin(); citr != this->screensSpaceComp.end(); ++citr) {
					citr->get()->GetShader()->Use();

					// Set view position
					glUniform3f(glGetUniformBlockIndex(citr->get()->GetShader()->GetProgram(), "viewPosW"), viewPosition.x, viewPosition.y, viewPosition.z);

					// For now, turn on ambient intensity and turn off lighting
					glUniform1f(glGetUniformLocation(citr->get()->GetShader()->GetProgram(), "ambLightIntensity"), 0.05f);
					glUniform1f(glGetUniformLocation(citr->get()->GetShader()->GetProgram(), "diffuseLightIntensity"), 0.0f);
					glUniform1f(glGetUniformLocation(citr->get()->GetShader()->GetProgram(), "specularLightIntensity"), 0.0f);
					citr->get()->Render(&viewMatrix[0][0], &this->ProjectionMatrix[0][0]);
			}
			// Remove blending
			glDisable(GL_BLEND);

			} // for renderstage ... avoid diffs

			// Unbind frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// Stereo post processing (for Rift)
			if(renderMode == GLS_RIFT && this->renderTargets.size() > 0) {
				glViewport(0, 0, windowWidth, windowHeight);
				glDepthFunc(GL_ALWAYS);
				glDisable(GL_CULL_FACE);
				glBindVertexArray(this->multipassVAO);
				this->multipassShader->Use();
				glUniform1i((*multipassShader)("in_Texture"), 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_id);
				GLuint pgm = this->multipassShader->GetProgram();
				glUniform2f(glGetUniformLocation(pgm, "ScaleIn"), riftScaleIn.x, riftScaleIn.y);
				glUniform2f(glGetUniformLocation(pgm, "ScaleOut"), riftScaleOut.x, riftScaleOut.y);
				glUniform4f(glGetUniformLocation(pgm, "kfact"), riftDistortionK.x, riftDistortionK.y, riftDistortionK.z, riftDistortionK.w);
				glUniform4f(glGetUniformLocation(pgm, "cfact"), riftChromaK.x, riftChromaK.y, riftChromaK.z, riftChromaK.w);
				glUniform4f(glGetUniformLocation(pgm, "LensCenter"), riftLensCenterL.x, riftLensCenterL.y, riftLensCenterR.x, riftLensCenterR.y);
				glUniform4f(glGetUniformLocation(pgm, "ScreenCenter"), riftScreenCenterL.x, riftScreenCenterL.y, riftScreenCenterR.x, riftScreenCenterR.y);
				
				glDrawArrays(GL_TRIANGLE_STRIP,0,4);
				// done
				this->multipassShader->UnUse();
				glBindVertexArray(0);
				glEnable(GL_CULL_FACE);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDepthFunc(GL_LESS);
				glFinish();
			}

            this->deltaAccumulator = 0.0;
            return true;
        }
        return false;
    }

	GLTransform *OpenGLSystem::GetTransformFor(const unsigned int entityID) {
		auto entity = &(_Components[entityID]);

		// for now, just returns the first component's transform
		// bigger question: should entities be able to have multiple GLComponents?
		for(auto compItr = entity->begin(); compItr != entity->end(); compItr++) {
			IGLComponent *glComp = dynamic_cast<IGLComponent *>((*compItr).second.get());
			if(glComp) {
				GLTransform *transform = glComp->Transform();
				return transform;
			}
		}

		// no GL components
		return 0;
	}

    const int* OpenGLSystem::Start() {
        // Use the GL3 way to get the version number
        glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
        glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

		// Sanity check to make sure we are at least in a good major version number.
		assert((OpenGLVersion[0] > 1) && (OpenGLVersion[0] < 5));

		// Determine the aspect ratio and sanity check it to a safe ratio
		float aspectRatio = static_cast<float>(this->windowWidth) / static_cast<float>(this->windowHeight);
		if (aspectRatio < 1.0f) {
			aspectRatio = 4.0f / 3.0f;
		}

		// Generate a projection matrix (the "view") based on basic window dimensions
        this->ProjectionMatrix = glm::perspective(
            45.0f, // field-of-view (height)
            aspectRatio, // aspect ratio
            0.1f, // near culling plane
            10000.0f // far culling plane
            );

        // App specific global gl settings
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		if (GLEW_AMD_seamless_cubemap_per_texture) {
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // allows for cube-mapping without seams
		}
		if (GLEW_ARB_multisample) {
			glEnable(GL_MULTISAMPLE_ARB);
		}
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

		// Hack in a full screen quad for post processing
		this->multipassShader = new GLSLShader();
		std::string vertFilename = "shaders/post.vert";
        std::string fragFilename = "shaders/post.frag";
        GLSLShader* theShader = new GLSLShader();
        this->multipassShader->LoadFromFile(GL_VERTEX_SHADER, vertFilename);
        this->multipassShader->LoadFromFile(GL_FRAGMENT_SHADER, fragFilename);
		this->multipassShader->CreateAndLinkProgram();
		Vertex allscreenverts[4] = {
			Vertex(-1.0f,-1.0f, 0.0f),Vertex(1.0f,-1.0f, 0.0f),
			Vertex(-1.0f, 1.0f, 0.0f),Vertex(1.0f, 1.0f, 0.0f)
		};
		TexCoord allscreenuv[4] = {
			TexCoord(0.0f,0.0f), TexCoord(1.0f,0.0f),
			TexCoord(0.0f,1.0f), TexCoord(1.0f,1.0f)
		};
		glGenVertexArrays(1, &this->multipassVAO); // Generate a VAO
        glBindVertexArray(this->multipassVAO); // Bind the VAO

        glGenBuffers(2, &this->multipassBuffers[0]); // Generate a vertex buffer.
        glBindBuffer(GL_ARRAY_BUFFER, this->multipassBuffers[0]); // Bind the vertex buffer.
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, allscreenverts, GL_STATIC_DRAW); // Stores the verts in the vertex buffer.
        GLint posLocation = glGetAttribLocation(this->multipassShader->GetProgram(), "in_Position");
		glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(posLocation);
		glBindBuffer(GL_ARRAY_BUFFER, this->multipassBuffers[1]); // Bind the UV buffer.
        glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * 4, allscreenuv, GL_STATIC_DRAW); // Stores the verts in the vertex buffer.
        GLint uvLocation = glGetAttribLocation(this->multipassShader->GetProgram(), "in_UV");
		glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(uvLocation);
		glBindVertexArray(0); // unbind when done
		// Create main framebuffer (index 0)
		//this->createRenderTarget(1024, 768, GL_RGBA8);

        return OpenGLVersion;
    }

    void OpenGLSystem::SetViewportSize(const unsigned int width, const unsigned int height) {
        this->windowHeight = height;
		this->windowWidth = width;

		// Determine the aspect ratio and sanity check it to a safe ratio
		float aspectRatio = static_cast<float>(this->windowWidth) / static_cast<float>(this->windowHeight);
		if (aspectRatio < 1.0f) {
			aspectRatio = 4.0f / 3.0f;
		}

        // update projection matrix based on new aspect ratio
        this->ProjectionMatrix = glm::perspective(
            45.0f,
            aspectRatio,
            0.1f,
            10000.0f
            );
    }
} // namespace Sigma
