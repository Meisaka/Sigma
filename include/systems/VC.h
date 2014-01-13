#pragma once
#ifndef _VC_SYSTEM_H_
#define _VC_SYSTEM_H_ 1

#include "vm/include/VM.hpp"

#include "IFactory.h"
#include "ISystem.h"
#include "components/VCMotherboard.h"
#include "components/IVCDevice.h"
#include "Sigma.h"

namespace Sigma {

	class VCSystem : public IFactory, public ISystem<IComponent> {

		DLL_EXPORT VCSystem ();
		DLL_EXPORT virtual ~VCSystem ();

		/**
		 * \brief Starts the Virtual Computer audio system.
		 * \return false on failure, true otherwise.
		 */
		DLL_EXPORT bool Start();

		/**
		 * \brief Updates every Virtual Computers runin in the system
		 * \return true if any audio processing was performed
		 */
		DLL_EXPORT bool Update();

		/**
		 * \brief Returns the list of Factory functions and types they create
		 *
		 *
		 * \return std::map<std::string, FactoryFunction> Contains Callbacks for different Component types that can be created by this class
		 */
		std::map<std::string,FactoryFunction> getFactoryFunctions();
	
		DLL_EXPORT IComponent* createCDADevice(const id_t entityID, const std::vector<Property> &properties);

	};


} // End of namespace Sigma

#endif // _VC_SYSTEM_H_

