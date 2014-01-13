#include "systems/VC.h"

#include "components/IVCDevice.h"
#include "Property.h"
#include "IComponent.h"


namespace Sigma {

	DLL_EXPORT VCSystem::VCSystem () {
	}

	DLL_EXPORT VCSystem::~VCSystem () {
	}

	DLL_EXPORT bool VCSystem::Start () {
	}

	DLL_EXPORT bool VCSystem::Update () {
	}

	std::map<std::string,Sigma::IFactory::FactoryFunction> VCSystem::getFactoryFunctions() {
		using namespace std::placeholders;
		std::map<std::string,Sigma::IFactory::FactoryFunction> retval;
		retval["CDADevice"] = std::bind(&VCSystem::createCDADevice,this,_1,_2);

		return retval;
	}

	DLL_EXPORT IComponent* VCSystem::createCDADevice(const id_t entityID, const std::vector<Property> &properties) {
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


		IVCDevice* dev = new IVCDevice(entityID);
		dev->SetJmp1(jmp1);
		dev->SetJmp2(jmp2);

		return dev;
	}

} // End of namespace Sigma

