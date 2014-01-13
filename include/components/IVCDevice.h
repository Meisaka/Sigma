#pragma once

#ifndef _IVC_DEVICE_COMPONENT_H_
#define _IVC_DEVICE_COMPONENT_H_ 1

#include "vm/include/VM.hpp"
#include "../IComponent.h"
#include "Sigma.h"

namespace Sigma {

	class IVCDevice : public IComponent {
	public:

		IVCDevice (const id_t id = 0) : IComponent(id) {
		}

		virtual ~IVCDevice () {
		}

		SET_COMPONENT_TYPENAME("VCDevice");

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

} // End of namespace Sigma

#endif // _VC_DEVICE_COMPONENT_H_

