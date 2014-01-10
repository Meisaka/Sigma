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
#include "components/GLScreenCursor.h"
#include "components/PointLight.h"
#include "components/SpotLight.h"

#ifdef __APPLE__
// Do not include <OpenGL/glu.h> because that will include gl.h which will mask all sorts of errors involving the use of deprecated GL APIs until runtime.
// gluErrorString (and all of glu) is deprecated anyway (TODO).
extern "C" const GLubyte * gluErrorString (GLenum error);
#else
#include "GL/glew.h"
#endif

#include "glm/glm.hpp"
#include "glm/ext.hpp"

namespace Sigma{
	// RenderTarget methods
	RenderTarget::~RenderTarget() {
		glDeleteTextures(this->texture_ids.size(), &this->texture_ids[0]); // Perhaps should check if texture was created for this RT or is used elsewhere
		glDeleteRenderbuffers(1, &this->depth_id);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &this->fbo_id);
	}

	void RenderTarget::BindWrite() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fbo_id);

		std::vector<GLenum> buffers;

		for(unsigned int i=0; i < this->texture_ids.size(); i++) {
			buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		glDrawBuffers(this->texture_ids.size(), &buffers[0]);
	}

	void RenderTarget::BindRead() {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbo_id);
	}

	void RenderTarget::UnbindWrite() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		std::vector<GLenum> buffers;
		buffers.push_back(GL_COLOR_ATTACHMENT0);
		glDrawBuffers(1, &buffers[0]);
	}

	void RenderTarget::UnbindRead() {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}

	std::map<std::string, Sigma::resource::GLTexture> OpenGLSystem::textures;

/*<<<<<<< HEAD
	OpenGLSystem::OpenGLSystem() : windowWidth(1024), windowHeight(768), deltaAccumulator(0.0),
		framerate(60.0f), viewMode(""), renderMode(GLS_NONE) {}
=======*/
	OpenGLSystem::OpenGLSystem() : windowWidth(1024), windowHeight(768), deltaAccumulator(0.0),
		framerate(60.0f), pointQuad(1000), ambientQuad(1001), spotQuad(1002) {}


	std::map<std::string, Sigma::IFactory::FactoryFunction> OpenGLSystem::getFactoryFunctions() {
		using namespace std::placeholders;

		std::map<std::string, Sigma::IFactory::FactoryFunction> retval;
		retval["GLSprite"] = std::bind(&OpenGLSystem::createGLSprite,this,_1,_2);
		retval["GLIcoSphere"] = std::bind(&OpenGLSystem::createGLIcoSphere,this,_1,_2);
		retval["GLCubeSphere"] = std::bind(&OpenGLSystem::createGLCubeSphere,this,_1,_2);
		retval["GLMesh"] = std::bind(&OpenGLSystem::createGLMesh,this,_1,_2);
		retval["RiftCamera"] = std::bind(&OpenGLSystem::createGLView,this,_1,_2);
		retval["FPSCamera"] = std::bind(&OpenGLSystem::createGLView,this,_1,_2);
		retval["GLSixDOFView"] = std::bind(&OpenGLSystem::createGLView,this,_1,_2);
		retval["PointLight"] = std::bind(&OpenGLSystem::createPointLight,this,_1,_2);
		retval["SpotLight"] = std::bind(&OpenGLSystem::createSpotLight,this,_1,_2);
		retval["GLScreenQuad"] = std::bind(&OpenGLSystem::createScreenQuad,this,_1,_2);
		retval["GLScreenCursor"] = std::bind(&OpenGLSystem::createScreenCursor,this,_1,_2);

		return retval;
	}

	IComponent* OpenGLSystem::createGLView(const id_t entityID, const std::vector<Property> &properties) {
		this->views.push_back(new IGLView(entityID));

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

		this->views[this->views.size() - 1]->Transform()->TranslateTo(x,y,z);
		this->views[this->views.size() - 1]->Transform()->Rotate(rx,ry,rz);

		this->addComponent(entityID, this->views[this->views.size() - 1]);

		return this->views[this->views.size() - 1];
	}

	IComponent* OpenGLSystem::createGLSprite(const id_t entityID, const std::vector<Property> &properties) {
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

	IComponent* OpenGLSystem::createGLIcoSphere(const id_t entityID, const std::vector<Property> &properties) {
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

	IComponent* OpenGLSystem::createGLCubeSphere(const id_t entityID, const std::vector<Property> &properties) {
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

	IComponent* OpenGLSystem::createGLMesh(const id_t entityID, const std::vector<Property> &properties) {
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
			else if (p->GetName() == "shader") {
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

	IComponent* OpenGLSystem::createScreenQuad(const id_t entityID, const std::vector<Property> &properties) {
		Sigma::GLScreenQuad* quad = new Sigma::GLScreenQuad(entityID);

		float x = 0.0f;
		float y = 0.0f;
		float w = 0.0f;
		float h = 0.0f;

		int componentID = 0;
		std::string textureName;
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
				textureName = p->Get<std::string>();
				textureInMemory = true;
			}
			else if (p->GetName() == "textureFileName") {
				textureName = p->Get<std::string>();
			}
		}

		// Check if the texture is loaded and load it if not.
		if (textures.find(textureName) == textures.end()) {
			Sigma::resource::GLTexture texture;
			if (textureInMemory) { // We are using an in memory texture. It will be populated somewhere else
				Sigma::OpenGLSystem::textures[textureName] = texture;
			}
			else { // The texture in on disk so load it.
				texture.LoadDataFromFile(textureName);
				if (texture.GetID() != 0) {
					Sigma::OpenGLSystem::textures[textureName] = texture;
				}
			}
		}

		// It should be loaded, but in case an error occurred double check for it.
		if (textures.find(textureName) != textures.end()) {
			quad->SetTexture(&Sigma::OpenGLSystem::textures[textureName]);
		}

		quad->SetPosition(x, y);
		quad->SetSize(w, h);
		quad->LoadShader("shaders/quad");
		quad->InitializeBuffers();
		this->screensSpaceComp.push_back(std::unique_ptr<IGLComponent>(quad));

		return quad;
	}

	IComponent* OpenGLSystem::createScreenCursor(const id_t entityID, const std::vector<Property> &properties) {
		Sigma::event::handler::GLScreenCursor* cursor = new Sigma::event::handler::GLScreenCursor(entityID);

		float x = 0.0f;
		float y = 0.0f;
		float w = 0.0f;
		float h = 0.0f;
		float hotspotx = 0.0f;
		float hotspoty = 0.0f;
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
			else if (p->GetName() == "hotspotx") {
				hotspotx = p->Get<float>();
			}
			else if (p->GetName() == "hotspoty") {
				hotspoty = p->Get<float>();
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
			cursor->SetTexture(&Sigma::OpenGLSystem::textures[textrueName]);
		}
		cursor->SetHotspot(hotspotx, hotspoty);
		cursor->SetPosition(x, y);
		cursor->SetSize(w, h);
		cursor->LoadShader("shaders/cursor");
		cursor->InitializeBuffers();
		this->screensSpaceComp.push_back(std::unique_ptr<IGLComponent>(cursor));
		return cursor;
	}

	IComponent* OpenGLSystem::createPointLight(const id_t entityID, const std::vector<Property> &properties) {
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

	IComponent* OpenGLSystem::createSpotLight(const id_t entityID, const std::vector<Property> &properties) {
		Sigma::SpotLight *light = new Sigma::SpotLight(entityID);

		float x=0.0f, y=0.0f, z=0.0f;
		float rx=0.0f, ry=0.0f, rz=0.0f;

		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property*  p = &*propitr;
			if (p->GetName() == "x") {
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
			else if (p->GetName() == "innerAngle") {
				light->innerAngle = p->Get<float>();
				light->cosInnerAngle = glm::cos(light->innerAngle);
			}
			else if (p->GetName() == "outerAngle") {
				light->outerAngle = p->Get<float>();
				light->cosOuterAngle = glm::cos(light->outerAngle);
			}
		}

		light->transform.TranslateTo(x, y, z);
		light->transform.Rotate(rx, ry, rz);

		this->addComponent(entityID, light);

		return light;
	}

/*<<<<<<< HEAD
	int OpenGLSystem::createRenderTarget(...) {
...
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
*/
	int OpenGLSystem::createRenderTarget(const unsigned int w, const unsigned int h, bool hasDepth) {
		std::unique_ptr<RenderTarget> newRT(new RenderTarget());

		newRT->width = w;
		newRT->height = h;
		newRT->hasDepth = hasDepth;

		this->renderTargets.push_back(std::move(newRT));
		return (this->renderTargets.size() - 1);
	}

	void OpenGLSystem::initRenderTarget(unsigned int rtID) {
		RenderTarget *rt = this->renderTargets[rtID].get();

		// Make sure we're on the back buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Get backbuffer depth bit width
		int depthBits;
#ifdef __APPLE__
		// The modern way.
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits);
#else
		glGetIntegerv(GL_DEPTH_BITS, &depthBits);
#endif

		// Create the depth render buffer
		if(rt->hasDepth) {
			glGenRenderbuffers(1, &rt->depth_id);
			glBindRenderbuffer(GL_RENDERBUFFER, rt->depth_id);

			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, rt->width, rt->height);
			printOpenGLError();

			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}

		// Create the frame buffer object
		glGenFramebuffers(1, &rt->fbo_id);
		printOpenGLError();

		glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo_id);

		for(unsigned int i=0; i < rt->texture_ids.size(); ++i) {
			//Attach 2D texture to this FBO
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, rt->texture_ids[i], 0);
			printOpenGLError();
		}

		if(rt->hasDepth) {
			//Attach depth buffer to FBO
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->depth_id);
			printOpenGLError();
		}

/*<<<< HEAD
		glGenFramebuffers(1, &newRT->fbo_id);
		glGenFramebuffers(1, &newRT->rsfbo_id);

		glBindFramebuffer(GL_FRAMEBUFFER, newRT->rsfbo_id);
		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newRT->texture_id, 0);
		
		glBindFramebuffer(GL_FRAMEBUFFER, newRT->fbo_id);
		glGenRenderbuffers(1, &newRT->depth_id);
		glGenRenderbuffers(1, &newRT->color_id);

		// Create color buffer
		glBindRenderbuffer(GL_RENDERBUFFER, newRT->color_id);
		//glRenderbufferStorage(GL_RENDERBUFFER, format, w, h);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 16, format, w, h);
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, newRT->color_id);
		
		// Create depth buffer
		glBindRenderbuffer(GL_RENDERBUFFER, newRT->depth_id);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 16, GL_DEPTH_COMPONENT24, w, h);
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, newRT->depth_id);
		
===*/
		//Does the GPU support current FBO configuration?
		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		switch(status) {
		case GL_FRAMEBUFFER_COMPLETE:
			std::cout << "Successfully created render target.\n";
			break;
		default:
			assert(0 && "Error: Framebuffer format is not compatible.");
		}

		// Unbind objects
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLSystem::createRTBuffer(unsigned int rtID, GLint format, GLenum internalFormat, GLenum type) {
		RenderTarget *rt = this->renderTargets[rtID].get();

		// Create a texture for each requested target
		GLuint texture_id;

		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		// Texture params for full screen quad
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//NULL means reserve texture memory, but texels are undefined
		glTexImage2D(GL_TEXTURE_2D, 0, format,
					 (GLsizei)rt->width,
					 (GLsizei)rt->height,
					 0, internalFormat, type, NULL);

		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+rt->texture_ids.size(), GL_TEXTURE_2D, texture_id, 0);

		this->renderTargets[rtID]->texture_ids.push_back(texture_id);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	bool OpenGLSystem::Update(const double delta) {
		this->deltaAccumulator += delta;

		// Check if the deltaAccumulator is greater than 1/<framerate>th of a second.
		//  ..if so, it's time to render a new frame
		if (this->deltaAccumulator > (1.0 / this->framerate)) {

			/////////////////////
			// Rendering Setup //
			/////////////////////

			glm::vec3 viewPosition;
			glm::mat4 viewProjInv;
/*<<<<<<< HEAD
			if(renderMode != GLS_NONE && this->renderTargets.size() > 0) {
				// Bind the primary render target
				glBindFramebuffer(GL_FRAMEBUFFER, this->renderTargets[0]->fbo_id);
			}

			//glm::vec3 viewPosition;
			glm::mat4 viewMatrix;
			glm::mat4 viewFixMatrix;
			glm::mat4 viewInfMatrix;
			glm::mat4 viewProj;
			glm::mat4 ProjMatrix;
			
			int looprender;
			int renderstage;
			bool renderstereo = false;
			if(renderMode == GLS_RIFT) {
				looprender = 1;
				renderstereo = true;
			}
			else { looprender = 0; }

			glClearColor(0.0f,0.0f,0.0f,0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers
			for(renderstage = 0; renderstage <= looprender; renderstage++) {
			// Set up the scene to a "clean" state.

			if(renderMode == GLS_RIFT && this->renderTargets.size() > 0) {
				glEnable(GL_MULTISAMPLE);
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
						viewFixMatrix = this->views[this->views.size() - 1]->GetViewMatrix(VIEW_FIXED_LEFT,stereoViewIPD*0.01f);
						//viewPosition = this->views[this->views.size() - 1]->Transform.GetPosition();
					}
					//viewMatrix = glm::translate(viewMatrix,-stereoViewLeft,0.0f,0.0f);
					viewFixMatrix = glm::translate(viewFixMatrix,0.0f,0.0f,-2.2f);
					viewProj = viewMatrix;
					ProjMatrix = this->stereoProjectionLeft;
					viewProj = ProjMatrix * viewProj;
				}
				else {
					if (this->views.size() > 0) {
						viewMatrix = this->views[this->views.size() - 1]->GetViewMatrix(VIEW_RIGHT,stereoViewIPD);
						viewFixMatrix = this->views[this->views.size() - 1]->GetViewMatrix(VIEW_FIXED_RIGHT,stereoViewIPD*0.01f);
						//viewPosition = this->views[this->views.size() - 1]->Transform.GetPosition();
					}
					//viewMatrix = glm::translate(viewMatrix,-stereoViewRight,0.0f,0.0f);
					viewFixMatrix = glm::translate(viewFixMatrix,0.0f,0.0f,-2.2f);
					viewProj = viewMatrix;
					ProjMatrix = this->stereoProjectionRight;
					viewProj = ProjMatrix * viewProj;
				}
=======*/

			// Setup the view matrix and position variables
			glm::mat4 viewMatrix;
			if (this->views.size() > 0) {
				viewMatrix = this->views[this->views.size() - 1]->GetViewMatrix();
				viewPosition = this->views[this->views.size() - 1]->Transform()->GetPosition();
			}
			else {
				if(this->renderTargets.size() > 0) {
					glViewport(0, 0, stereoFBTw, stereoFBTh); // Set the viewport size to fill the framebuffer
				}
				else {
					glViewport(0, 0, windowWidth, windowHeight); // Set the viewport size to fill the window
				}
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

/*<<<<<<< HEAD
				if (this->views.size() > 0) {
					viewInfMatrix = this->views[this->views.size() - 1]->GetViewMatrix();
					//viewPosition = this->views[this->views.size() - 1]->Transform.GetPosition();
				}
				viewProj = viewMatrix = viewInfMatrix;
				viewProj *= this->ProjectionMatrix;
				ProjMatrix= this->ProjectionMatrix;
			}
=======*/
			// Setup the projection matrix
			glm::mat4 viewProj = glm::mul(this->ProjectionMatrix, viewMatrix);

			viewProjInv = glm::inverse(viewProj);

			// Calculate frustum for culling
			this->GetView(0)->CalculateFrustum(viewProj);

			// Clear the backbuffer and primary depth/stencil buffer
			glClearColor(0.0f,0.0f,0.0f,1.0f);
			glViewport(0, 0, this->windowWidth, this->windowHeight); // Set the viewport size to fill the window
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

			//////////////////
			// GBuffer Pass //
			//////////////////

			// Bind the first buffer, which is the Geometry Buffer
			if(this->renderTargets.size() > 0) {
				this->renderTargets[0]->BindWrite();
			}

			// Disable blending
			glDisable(GL_BLEND);

			// Clear the GBuffer
			glClearColor(0.0f,0.0f,0.0f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

			// Loop through and draw each GL Component component.
			for (auto eitr = this->_Components.begin(); eitr != this->_Components.end(); ++eitr) {
				for (auto citr = eitr->second.begin(); citr != eitr->second.end(); ++citr) {
					IGLComponent *glComp = dynamic_cast<IGLComponent *>(citr->second.get());

					if(glComp && glComp->IsLightingEnabled()) {
						glComp->GetShader()->Use();

						// Set view position
						//glUniform3f(glGetUniformBlockIndex(glComp->GetShader()->GetProgram(), "viewPosW"), viewPosition.x, viewPosition.y, viewPosition.z);

						// For now, turn on ambient intensity and turn off lighting
						glUniform1f(glGetUniformLocation(glComp->GetShader()->GetProgram(), "ambLightIntensity"), 0.05f);
						glUniform1f(glGetUniformLocation(glComp->GetShader()->GetProgram(), "diffuseLightIntensity"), 0.0f);
						glUniform1f(glGetUniformLocation(glComp->GetShader()->GetProgram(), "specularLightIntensity"), 0.0f);
/*<< HEAD
						if(glComp->IsInfiniteDistance()) {
							glComp->Render(&viewInfMatrix[0][0], &ProjMatrix[0][0]);
						}
						else {
							glComp->Render(&viewMatrix[0][0], &ProjMatrix[0][0]);
						}
*/
						glComp->Render(&viewMatrix[0][0], &this->ProjectionMatrix[0][0]);
					}
				}
			}

			// Unbind the first buffer, which is the Geometry Buffer
			if(this->renderTargets.size() > 0) {
				this->renderTargets[0]->UnbindWrite();
			}

			// Copy gbuffer's depth buffer to the screen depth buffer
			// needed for non deferred rendering at the end of this method
			// NOTE: I'm sure there's a faster way to do this
			if(this->renderTargets.size() > 0) {
				this->renderTargets[0]->BindRead();
			}

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, this->windowWidth, this->windowHeight, 0, 0, this->windowWidth, this->windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

			if(this->renderTargets.size() > 0) {
				this->renderTargets[0]->UnbindRead();
			}

			///////////////////
			// Lighting Pass //
			///////////////////

			// Disable depth testing
			glDepthFunc(GL_NONE);
			glDepthMask(GL_FALSE);

			// Bind the Geometry buffer for reading
			if(this->renderTargets.size() > 0) {
				this->renderTargets[0]->BindRead();
			}

			// Ambient light pass

			// Ensure that blending is disabled
			glDisable(GL_BLEND);

			// Currently simple constant ambient light, could use SSAO here
			glm::vec4 ambientLight(0.1f, 0.1f, 0.1f, 1.0f);

			GLSLShader &shader = (*this->ambientQuad.GetShader().get());
			shader.Use();

			// Load variables
			glUniform4f(shader("ambientColor"), ambientLight.r, ambientLight.g, ambientLight.b, ambientLight.a);
			glUniform1i(shader("colorBuffer"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_ids[0]);

			this->ambientQuad.Render(&viewMatrix[0][0], &this->ProjectionMatrix[0][0]);

			shader.UnUse();

			// Dynamic light passes
			// Turn on additive blending
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);

			// Loop through each light, render a fullscreen quad if it is visible
			for(auto eitr = this->_Components.begin(); eitr != this->_Components.end(); ++eitr) {
				for (auto citr = eitr->second.begin(); citr != eitr->second.end(); ++citr) {
					// Check if this component is a point light
					PointLight *light = dynamic_cast<PointLight*>(citr->second.get());

					// If it is a point light, and it intersects the frustum, then render
					if(light && this->GetView(0)->CameraFrustum.intersectsSphere(light->position, light->radius) ) {

						GLSLShader &shader = (*this->pointQuad.GetShader().get());
						shader.Use();

						// Load variables
						glUniform3fv(shader("viewPosW"), 1, &viewPosition[0]);
						glUniformMatrix4fv(shader("viewProjInverse"), 1, false, &viewProjInv[0][0]);
						glUniform3fv(shader("lightPosW"), 1, &light->position[0]);
						glUniform1f(shader("lightRadius"), light->radius);
						glUniform4fv(shader("lightColor"), 1, &light->color[0]);

						glUniform1i(shader("diffuseBuffer"), 0);
						glUniform1i(shader("normalBuffer"), 1);
						glUniform1i(shader("depthBuffer"), 2);

						// Bind GBuffer textures
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_ids[0]);
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_ids[1]);
						glActiveTexture(GL_TEXTURE2);
						glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_ids[2]);

						this->pointQuad.Render(&viewMatrix[0][0], &this->ProjectionMatrix[0][0]);

						shader.UnUse();

						continue;
					}

					SpotLight *spotLight = dynamic_cast<SpotLight *>(citr->second.get());

					if(spotLight && spotLight->IsEnabled()) {
						GLSLShader &shader = (*this->spotQuad.GetShader().get());
						shader.Use();

						glm::vec3 position = spotLight->transform.ExtractPosition();
						glm::vec3 direction = spotLight->transform.GetForward();

						// Load variables
						glUniform3fv(shader("viewPosW"), 1, &viewPosition[0]);
						glUniformMatrix4fv(shader("viewProjInverse"), 1, false, &viewProjInv[0][0]);
						glUniform3fv(shader("lightPosW"), 1, &position[0]);
						glUniform3fv(shader("lightDirW"), 1, &direction[0]);
						glUniform4fv(shader("lightColor"), 1, &spotLight->color[0]);
						glUniform1f(shader("lightCosInnerAngle"), spotLight->cosInnerAngle);
						glUniform1f(shader("lightCosOuterAngle"), spotLight->cosOuterAngle);

						glUniform1i(shader("diffuseBuffer"), 0);
						glUniform1i(shader("normalBuffer"), 1);
						glUniform1i(shader("depthBuffer"), 2);

						// Bind GBuffer textures
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_ids[0]);
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_ids[1]);
						glActiveTexture(GL_TEXTURE2);
						glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_ids[2]);

						this->spotQuad.Render(&viewMatrix[0][0], &this->ProjectionMatrix[0][0]);

						shader.UnUse();

						continue;
					}
				}
			}

			// Unbind the Geometry buffer for reading
			if(this->renderTargets.size() > 0) {
				this->renderTargets[0]->UnbindRead();
			}

			// Remove blending
			glDisable(GL_BLEND);

			// Re-enabled depth test
			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);

			////////////////////
			// Composite Pass //
			////////////////////

			// Not needed yet

			///////////////////////
			// Draw Unlit Objects
			///////////////////////

			// Loop through and draw each GL Component component.
			for (auto eitr = this->_Components.begin(); eitr != this->_Components.end(); ++eitr) {
				for (auto citr = eitr->second.begin(); citr != eitr->second.end(); ++citr) {
					IGLComponent *glComp = dynamic_cast<IGLComponent *>(citr->second.get());

					if(glComp && !glComp->IsLightingEnabled()) {
						glComp->GetShader()->Use();

						// Set view position
						glUniform3f(glGetUniformBlockIndex(glComp->GetShader()->GetProgram(), "viewPosW"), viewPosition.x, viewPosition.y, viewPosition.z);

/*<<<<<<< HEAD
						if(glComp->IsInfiniteDistance()) {
							glComp->Render(&viewInfMatrix[0][0], &ProjMatrix[0][0]);
						}
						else {
							glComp->Render(&viewMatrix[0][0], &ProjMatrix[0][0]);
						}
=======*/
						glComp->Render(&viewMatrix[0][0], &this->ProjectionMatrix[0][0]);
					}
				}
			}

			//////////////////
			// Overlay Pass //
			//////////////////

			// Enable transparent rendering
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
/*<<<<<<< HEAD
			for (auto citr = this->screensSpaceComp.begin(); citr != this->screensSpaceComp.end(); ++citr) {
				citr->get()->GetShader()->Use();
				glUniform1i((*citr->get()->GetShader())("enable_projection"), (renderstereo ? 1 : 0));
				citr->get()->Render(&viewFixMatrix[0][0], &ProjMatrix[0][0]);
=======*/

			for (auto citr = this->screensSpaceComp.begin(); citr != this->screensSpaceComp.end(); ++citr) {
				citr->get()->GetShader()->Use();
				citr->get()->Render(&viewMatrix[0][0], &this->ProjectionMatrix[0][0]);
			}

			// Remove blending
			glDisable(GL_BLEND);

			} // for renderstage ... avoid diffs

			// Unbind frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

/*<<<<<<< HEAD
			// Stereo post processing (for Rift)
			if(renderMode == GLS_RIFT && this->renderTargets.size() > 0) {
				glBindFramebuffer( GL_READ_FRAMEBUFFER, this->renderTargets[0]->fbo_id);
				glBindFramebuffer( GL_DRAW_FRAMEBUFFER, this->renderTargets[0]->rsfbo_id);
				glBlitFramebuffer( 0, 0, this->stereoFBTw, this->stereoFBTh, 0, 0, this->stereoFBTw, this->stereoFBTh, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(0, 0, windowWidth, windowHeight);
				glDepthFunc(GL_ALWAYS);
				glDisable(GL_CULL_FACE);
				glBindVertexArray(this->multipassVAO);
				this->multipassShader->Use();
				glUniform1i((*multipassShader)("in_Texture"), 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, this->renderTargets[0]->texture_id);
				glGenerateMipmap(GL_TEXTURE_2D);
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
				glDisable(GL_MULTISAMPLE);
				glFinish();
			}
*/
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
#if __APPLE__
		// GL_TEXTURE_CUBE_MAP_SEAMLESS and GL_MULTISAMPLE are Core.
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // allows for cube-mapping without seams
		glEnable(GL_MULTISAMPLE);
#else
		if (GLEW_AMD_seamless_cubemap_per_texture) {
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // allows for cube-mapping without seams
		}
		if (GLEW_ARB_multisample) {
			glEnable(GL_MULTISAMPLE_ARB);
		}
#endif
/*<<<<<<< HEAD
		// This really needs a FullScreenQuad or similar for all the multipass stuff
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
=======*/
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		// Setup a screen quad for deferred rendering
		this->pointQuad.SetSize(1.0f, 1.0f);
		this->pointQuad.SetPosition(0.0f, 0.0f);
		this->pointQuad.LoadShader("shaders/pointlight");
		this->pointQuad.Inverted(true);
		this->pointQuad.InitializeBuffers();
		this->pointQuad.SetCullFace("none");

		this->pointQuad.GetShader()->Use();
		this->pointQuad.GetShader()->AddUniform("viewPosW");
		this->pointQuad.GetShader()->AddUniform("viewProjInverse");
		this->pointQuad.GetShader()->AddUniform("lightPosW");
		this->pointQuad.GetShader()->AddUniform("lightRadius");
		this->pointQuad.GetShader()->AddUniform("lightColor");
		this->pointQuad.GetShader()->AddUniform("diffuseBuffer");
		this->pointQuad.GetShader()->AddUniform("normalBuffer");
		this->pointQuad.GetShader()->AddUniform("depthBuffer");
		this->pointQuad.GetShader()->UnUse();

		this->spotQuad.SetSize(1.0f, 1.0f);
		this->spotQuad.SetPosition(0.0f, 0.0f);
		this->spotQuad.LoadShader("shaders/spotlight");
		this->spotQuad.Inverted(true);
		this->spotQuad.InitializeBuffers();
		this->spotQuad.SetCullFace("none");

		this->spotQuad.GetShader()->Use();
		this->spotQuad.GetShader()->AddUniform("viewPosW");
		this->spotQuad.GetShader()->AddUniform("viewProjInverse");
		this->spotQuad.GetShader()->AddUniform("lightPosW");
		this->spotQuad.GetShader()->AddUniform("lightDirW");
		this->spotQuad.GetShader()->AddUniform("lightColor");
		this->spotQuad.GetShader()->AddUniform("lightCosInnerAngle");
		this->spotQuad.GetShader()->AddUniform("lightCosOuterAngle");
		this->spotQuad.GetShader()->AddUniform("diffuseBuffer");
		this->spotQuad.GetShader()->AddUniform("normalBuffer");
		this->spotQuad.GetShader()->AddUniform("depthBuffer");
		this->spotQuad.GetShader()->UnUse();

		this->ambientQuad.SetSize(1.0f, 1.0f);
		this->ambientQuad.SetPosition(0.0f, 0.0f);
		this->ambientQuad.LoadShader("shaders/ambient");
		this->ambientQuad.Inverted(true);
		this->ambientQuad.InitializeBuffers();
		this->ambientQuad.SetCullFace("none");

		this->ambientQuad.GetShader()->Use();
		this->ambientQuad.GetShader()->AddUniform("ambientColor");
		this->ambientQuad.GetShader()->AddUniform("colorBuffer");
		this->ambientQuad.GetShader()->UnUse();

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

//-----------------------------------------------------------------
// Print for OpenGL errors
//
// Returns 1 if an OpenGL error occurred, 0 otherwise.
//

int printOglError(const std::string &file, int line) {
	GLenum glErr;
	int retCode = 0;

	glErr = glGetError();
	if (glErr != GL_NO_ERROR) {
		std::cerr << "glError in file " << file << " @ line " << line << ": " << gluErrorString(glErr) << std::endl;
		retCode = 1;
	}
	return retCode;
}
