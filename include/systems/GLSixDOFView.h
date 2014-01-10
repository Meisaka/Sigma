#pragma once

#include "IGLView.h"
#include "Sigma.h"

namespace Sigma {
	struct GLSixDOFView : public Sigma::IGLView {

		SET_COMPONENT_TYPENAME("SIX_DOF_VIEW");

		GLSixDOFView(const id_t entityID);

		const glm::mat4 GetViewMatrix();
		virtual const glm::mat4 GetViewMatrix(Sigma::ViewSelection, float ipd = 0.0f) { return GetViewMatrix(); }
		virtual void Move(float right, float up, float forward);
		virtual glm::vec3 Restrict(glm::vec3 rotation) { return rotation; }
	};
}
