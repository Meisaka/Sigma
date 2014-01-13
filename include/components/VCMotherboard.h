#pragma once

#ifndef _VC_MOTHERBOARD_COMPONENT_H_
#define _VC_MOTHERBOARD_COMPONENT_H_ 1

#include "vm/include/VM.hpp"
#include "../IComponent.h"
#include "Sigma.h"

namespace Sigma {

	class VCMotherBoard : public IComponent {
	public:

		VCMotherBoard (const id_t id = 0) : IComponent(id) {
		}

		virtual ~VCMotherBoard () {
		}

		SET_COMPONENT_TYPENAME("VCMotherBoard");

	};

} // End of namespace Sigma

#endif // _VC_MOTHERBOARD_COMPONENT_H_

