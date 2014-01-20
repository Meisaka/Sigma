#include "systems/VC.h"

#include "components/IVCDevice.h"
#include "components/VCMotherboard.h"
#include "Property.h"
#include "IComponent.h"


namespace Sigma {

	DLL_EXPORT VCSystem::VCSystem () : vms() {
	}

	DLL_EXPORT VCSystem::~VCSystem () {
	}

	DLL_EXPORT bool VCSystem::Start () {
		std::cerr << "Setting up Virtual Machines" << std::endl;

		return true;
	}

	DLL_EXPORT bool VCSystem::Update (const double delta) {
		return true;
	}

	std::map<std::string,Sigma::IFactory::FactoryFunction> VCSystem::getFactoryFunctions() {
		using namespace std::placeholders;
		std::map<std::string,Sigma::IFactory::FactoryFunction> retval;
		
		retval["VCMotherBoard"] = std::bind(&VCSystem::createVCMotherBoard,this,_1,_2);
		
		retval["CDADevice"] = std::bind(&VCSystem::createCDADevice,this,_1,_2);
		retval["GKeyboardDevice"] = std::bind(&VCSystem::createGKeyboardDevice,this,_1,_2);

		return retval;
	}

	DLL_EXPORT IComponent* VCSystem::createVCMotherBoard(const id_t entityID, const std::vector<Property> &properties) {

		VCMotherBoard* mbo = new VCMotherBoard(entityID);
		
		this->addComponent(entityID, mbo);

		return mbo;
	}


	DLL_EXPORT IComponent* VCSystem::createCDADevice(const id_t entityID, const std::vector<Property> &properties) {
		// Tmp vars
		vm::dword_t jmp1 = 0, jmp2 = 0;
		std::string textureName = "";

		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property* p = &(*propitr);

			if (p->GetName() == "jmp1") {
				jmp1 = p->Get<vm::dword_t>();
			}
			else if (p->GetName() == "jmp2") {
				jmp2 = p->Get<vm::dword_t>();
			}
			else if (p->GetName() == "textureName") {
				textureName = p->Get<std::string>();
			}
		}


		CDADevice* dev = new CDADevice(entityID);
		dev->SetJmp1(jmp1);
		dev->SetJmp2(jmp2);

		Sigma::resource::GLTexture texture;
		Sigma::OpenGLSystem::textures[textureName] = texture;
		Sigma::OpenGLSystem::textures[textureName].Format(GL_RGBA);
		Sigma::OpenGLSystem::textures[textureName].AutoGenMipMaps(false);
		Sigma::OpenGLSystem::textures[textureName].MinFilter(GL_LINEAR);
		Sigma::OpenGLSystem::textures[textureName].GenerateGLTexture(320,240);
		dev->SetTexture(&Sigma::OpenGLSystem::textures[textureName]);

		this->addComponent(entityID, dev);

		return dev;
	}

	DLL_EXPORT IComponent* VCSystem::createGKeyboardDevice(const id_t entityID, const std::vector<Property> &properties) {
		// Tmp vars
		vm::dword_t jmp1 = 0, jmp2 = 0;

		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property* p = &(*propitr);

			if (p->GetName() == "jmp1") {
				jmp1 = p->Get<vm::dword_t>();
			}
			else if (p->GetName() == "jmp2") {
				jmp2 = p->Get<vm::dword_t>();
			}
		}


		GKeyboardDevice* dev = new GKeyboardDevice(entityID);
		dev->SetJmp1(jmp1);
		dev->SetJmp2(jmp2);
		
		this->addComponent(entityID, dev);

		return dev;
	}

} // End of namespace Sigma

