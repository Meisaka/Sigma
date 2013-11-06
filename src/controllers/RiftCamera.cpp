
#include "controllers/RiftCamera.h"

namespace Sigma {
	namespace event {
		namespace handler {
			RiftCamera::RiftCamera(int entityID) : FPSCamera(entityID) {
				this->ovrHMD = 0;
			}
			bool RiftCamera::SetHMD(OVR::HMDDevice * thehmd) {
				if(thehmd && !this->ovrHMD) {
					this->ovrHMD = thehmd;
					this->ovrHMD->GetDeviceInfo(&this->ovrInfo);
					this->ovrSensor = this->ovrHMD->GetSensor();
					this->ovrSF.AttachToSensor(this->ovrSensor);
					this->ovrSF.SetYawCorrectionEnabled(true);
					this->ovrSF.SetPrediction(0.020f,true);
					return true;
				}
				else {
					return false;
				}
			}

			bool RiftCamera::ClearHMD() { return false; }

			const glm::mat4 RiftCamera::GetViewMatrix() {
				OVR::Quatf riftorient;
				if(this->ovrSF.IsPredictionEnabled()) {
					riftorient = this->ovrSF.GetPredictedOrientation();
				}
				else {
					riftorient = this->ovrSF.GetOrientation();
				}
				glm::quat vieworient = glm::quat(riftorient.x, riftorient.y, riftorient.z, riftorient.w);

				glm::mat4 viewMatrix = glm::rotate(glm::mat4(1.0f), this->Transform.GetPitch(), glm::vec3(1.0f, 0, 0));
				viewMatrix = glm::rotate(viewMatrix, -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				viewMatrix = glm::rotate(viewMatrix, 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				viewMatrix = viewMatrix * glm::mat4_cast(vieworient);
				viewMatrix = glm::rotate(viewMatrix, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				viewMatrix = glm::rotate(viewMatrix, 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				viewMatrix = glm::rotate(viewMatrix, 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				viewMatrix = glm::rotate(viewMatrix, this->Transform.GetYaw(), glm::vec3(0, 1.0f, 0));
				viewMatrix = glm::translate(viewMatrix, -1.0f * this->Transform.GetPosition());
				return viewMatrix;
			}
		}
	}
} // namespace Sigma::event::handler
