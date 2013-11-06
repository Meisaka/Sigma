#pragma once
#ifndef RIFTCAM_H
#define RIFTCAM_H

#include "controllers/FPSCamera.h"
#include <OVR.h>

namespace Sigma {
	namespace event {
		namespace handler {
			class RiftCamera : public FPSCamera {
			public:
				SET_COMPONENT_TYPENAME("RIFT_CAMERA");
				
				RiftCamera(int entityID);

				/**
				 * \brief Updates and returns the view matrix.
				 *
				 * \return const glm::mat4 The current view matrix.
				 */
				const glm::mat4 GetViewMatrix();

				bool SetHMD(OVR::HMDDevice *);

				bool ClearHMD();
			private:
				OVR::HMDDevice * ovrHMD;
				OVR::HMDInfo ovrInfo;
				OVR::SensorDevice * ovrSensor;
				OVR::SensorFusion ovrSF;
			};
		}
	}
} // namespace Sigma::event::handler

#endif // RIFTCAM_H
