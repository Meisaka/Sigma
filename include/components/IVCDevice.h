#pragma once

#ifndef _IVC_DEVICE_COMPONENT_H_
#define _IVC_DEVICE_COMPONENT_H_ 1

#include "vm/include/VM.hpp"

#include "../IComponent.h"
#include "resources/GLTexture.h"
#include "systems/OpenGLSystem.h"
#include "Sigma.h"

namespace Sigma {

	class IVC_Device : public IComponent {
	public:

		IVC_Device (const id_t id = 0) : IComponent(id) {
		}

		virtual ~IVC_Device () {
		}

		SET_COMPONENT_TYPENAME("VCDevice");

		// Jumpers Get/Setters

		void SetJmp1 (vm::dword_t val) {
			jmp1 = val;
		}

		vm::dword_t GetJmp1 () const {
			return jmp1;
		}

		void SetJmp2 (vm::dword_t val) {
			jmp2 = val;
		}

		vm::dword_t GetJmp2 () const {
			return jmp2;
		}

	private:
		vm::dword_t jmp1;
		vm::dword_t jmp2;
	};

	/**
	 * CDA Device component
	 */
	class CDADevice : public IVC_Device {
	public:

		CDADevice (const id_t id = 0) : IVC_Device(id), texture(nullptr) {
		}

		virtual ~CDADevice () {
		}

		SET_COMPONENT_TYPENAME("CDADevice");

		void SetTexture(Sigma::resource::GLTexture* texture) {
			this->texture = texture;
		}

		Sigma::resource::GLTexture* GetTexture() {
			return this->texture;
		}

	private:
		Sigma::resource::GLTexture* texture;

	};

	/**
	 * Keyboard Device component
	 */
	class GKeyboardDevice : public IVC_Device {
	public:

		GKeyboardDevice (const id_t id = 0) : IVC_Device(id) {
		}

		virtual ~GKeyboardDevice () {
		}

		SET_COMPONENT_TYPENAME("GKeyboardDevice");

	private:

	};


} // End of namespace Sigma

#endif // _VC_DEVICE_COMPONENT_H_

