
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

			const glm::mat4 RiftCamera::GetViewMatrix(ViewSelection V, float ipd) {
				OVR::Quatf riftorient;
				if(this->ovrSF.IsPredictionEnabled()) {
					riftorient = this->ovrSF.GetPredictedOrientation();
				}
				else {
					riftorient = this->ovrSF.GetOrientation();
				}
				glm::quat vieworient = glm::quat(riftorient.x, riftorient.y, riftorient.z, riftorient.w);
				float trans;
				switch(V) {
				case VIEW_LEFT:
					trans = ipd * 0.5f;
					break;
				case VIEW_RIGHT:
					trans = ipd * -0.5f;
					break;
				default:
					trans = 0.0f;
					break;
				}
				glm::mat4 viewMatrix = glm::mat4(1.0f);
				viewMatrix = glm::translate(trans,0.0f,0.0f) * viewMatrix;
				//viewMatrix = glm::rotate(viewMatrix, this->Transform.GetPitch(), glm::vec3(1.0f, 0, 0));
				viewMatrix = glm::rotate(viewMatrix, -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				viewMatrix = glm::rotate(viewMatrix, 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				viewMatrix = viewMatrix * glm::mat4_cast(vieworient);
				viewMatrix = glm::rotate(viewMatrix, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				viewMatrix = glm::rotate(viewMatrix, 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				viewMatrix = glm::rotate(viewMatrix, 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				viewMatrix = glm::rotate(viewMatrix, this->Transform.GetYaw(), glm::vec3(0, 1.0f, 0));
				if(V != VIEW_INFINITE) {
					viewMatrix = glm::translate(viewMatrix, -1.0f * this->Transform.GetPosition());
				}
				return viewMatrix;
			}

			const glm::mat4 RiftCamera::GetViewMatrix() {
				return RiftCamera::GetViewMatrix(VIEW_CENTER);
			}
		}
	}
} // namespace Sigma::event::handler