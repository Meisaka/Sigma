#pragma once
#ifndef _VC_SYSTEM_H_
#define _VC_SYSTEM_H_ 1

#include "vm/include/VM.hpp"

#include "IFactory.h"
#include "ISystem.h"
#include "systems/KeyboardInputSystem.h"
#include "components/VCMotherboard.h"
#include "components/IVCDevice.h"
#include "controllers/VirtualKeyboard.h"
#include "Sigma.h"

#include <unordered_map>

namespace Sigma {

	class VCSystem : public IFactory, public ISystem<IComponent> {
	public:
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
		DLL_EXPORT bool Update(const double delta);

		/**
		 * \brief Returns the list of Factory functions and types they create
		 *
		 *
		 * \return std::map<std::string, FactoryFunction> Contains Callbacks for different Component types that can be created by this class
		 */
		std::map<std::string,FactoryFunction> getFactoryFunctions();
	
		DLL_EXPORT IComponent* createVCMotherBoard(const id_t entityID, const std::vector<Property> &properties);
		
		DLL_EXPORT IComponent* createCDADevice(const id_t entityID, const std::vector<Property> &properties);

		DLL_EXPORT IComponent* createGKeyboardDevice(const id_t entityID, const std::vector<Property> &properties);
	
	private:
		std::unordered_map<id_t, vm::VirtualComputer<vm::cpu::TR3200>*> vms;	/// Container of VMs
		// TODO Really... Change this
		std::unordered_map<id_t, vm::cda::CDA*> cdas;	/// Container of graphic cardss
		
		std::unordered_map<id_t, vm::keyboard::GKeyboard*> gkeys;	/// Container of virtual keyboards

		double vsync_delta;
		vm::dword_t tbuffer[320*240];		/// Buffer to store the output screen

	public:
		std::unordered_map<id_t, Sigma::event::handler::VirtualKeyboard*> vkeys;	/// Container of virtual keyboards

	};


} // End of namespace Sigma

#endif // _VC_SYSTEM_H_

