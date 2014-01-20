#include "systems/VC.h"

#include "components/IVCDevice.h"
#include "components/VCMotherboard.h"
#include "Property.h"
#include "IComponent.h"


namespace Sigma {

	DLL_EXPORT VCSystem::VCSystem () : vms(), cdas() {
	}

	DLL_EXPORT VCSystem::~VCSystem () {
		for (auto it = vms.begin() ; it != vms.end() ; ++it) {
			delete it->second;
			it->second = nullptr;
		}

		for (auto it = cdas.begin() ; it != cdas.end() ; ++it) {
			delete it->second;
			it->second = nullptr;
		}
	}

	DLL_EXPORT bool VCSystem::Start () {
		std::cerr << "Setting up Virtual Machines" << std::endl;

		return true;
	}

	DLL_EXPORT bool VCSystem::Update (const double delta) {
		for (auto it = vms.begin() ; it != vms.end() ; ++it) {
			it->second->Tick(1000); // TODO Calc ticks in function of delta
		}
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
		// Tmp vars
		std::string romfile = "";
		
		for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
			const Property* p = &(*propitr);
			
			if (p->GetName() == "romfile") {
				romfile = p->Get<std::string>();
			}
		}

		VCMotherBoard* mbo = new VCMotherBoard(entityID);
		mbo->SetROMFileName(romfile);
		
		this->addComponent(entityID, mbo);
		vms.emplace(entityID, new vm::VirtualComputer<vm::cpu::TR3200>() );

		auto vm = vms[entityID];
		// Load ROM
		auto bytes = vm::aux::LoadROM(mbo->GetROMFileName(), *vm );
		std::cerr << "Loaded ROM File : " << mbo->GetROMFileName() << " With size : " << bytes << " bytes\n";

		// Load Devices (TODO: think how and were is stored every device)
		auto cdadev = (CDADevice*) this->getComponent(entityID, "CDADevice");
		if (cdadev != nullptr) {
			cdas.emplace(entityID, new vm::cda::CDA());
			vm->AddDevice(0, *(cdas[entityID]));
		}

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

