#pragma once

#include "systems/IGLView.h"
#include <OVR.h>

namespace Sigma {
	class RiftCamera : public IGLView {
	public:
		SET_COMPONENT_TYPENAME("RIFT_CAMERA");
				
		RiftCamera(int entityID);

		/**
			* \brief Updates and returns the view matrix.
			*
			* \return const glm::mat4 The current view matrix.
			*/
		const glm::mat4 GetViewMatrix();
		const glm::mat4 GetViewMatrix(ViewSelection, float ipd = 0.0f);

		bool SetHMD(OVR::HMDDevice *);

		const OVR::HMDInfo& GetHMDInfo() { return ovrInfo; }

		bool ClearHMD();
	private:
		OVR::HMDDevice * ovrHMD;
		OVR::HMDInfo ovrInfo;
		OVR::SensorDevice * ovrSensor;
		OVR::SensorFusion ovrSF;
	};
} // namespace Sigma

