#include "controllers/VirtualKeyboard.h"

#include "OS.h"

#include <iostream>

namespace Sigma {
	namespace event {
		namespace handler {

			VirtualKeyboard::VirtualKeyboard () : hasFocus(false), gkeyboard(nullptr), actionsound(nullptr) {
				// Register listened keys.
				for (unsigned i = 0; i <= 265 ; i++) { 
					this->keys.push_back(i);
				}
				this->keys.push_back(340);
				this->keys.push_back(341);
				this->keys.push_back(344);
				this->keys.push_back(345);
				this->keys.push_back(346);
				this->keys.push_back(GLFW_KEY_F3);
				this->keys.push_back(GLFW_KEY_F4);
			}

			void VirtualKeyboard::KeyStateChange(const unsigned int key, const KEY_STATE state) {
				KeyStateChange(key, state, KEY_STATE::KS_UP);
			}
			void VirtualKeyboard::KeyStateChange(const unsigned int key, const KEY_STATE state, const KEY_STATE laststate) {
				if (key == GLFW_KEY_F3 && state != KEY_STATE::KS_DOWN) { // F3 togles the keyboard focus
					this->hasFocus ^= 1;
					if(this->hasFocus & 1) {
						this->keyboardSystem->RequestFocusLock(this);
					}
					else {
						this->keyboardSystem->ReleaseFocusLock(this);
					}
					return;
				}

				if (this->gkeyboard != nullptr && (hasFocus & 1) && key != GLFW_KEY_F3) {
					auto keycode = vm::aux::GLFWKeyToTR3200(key);
					bool kdown = state == KEY_STATE::KS_DOWN;
					if(actionsound) {
						if(kdown && (laststate == KEY_STATE::KS_UP)) {
							actionsound->Play(PLAYBACK_NORMAL);
						}
					}
					if(reactionsound) {
						if(!kdown) {
							reactionsound->Play(PLAYBACK_NORMAL);
						}
					}
					if(key == GLFW_KEY_F4) {
						if(this->vcvm) {
							this->vcvm->Reset();
						}
					}
					else {
						this->gkeyboard->PushKeyEvent(kdown, keycode);
					}
				}
			}

			void VirtualKeyboard::LostKeyboardFocus() {
			}

		} // End of namespace handler
	} // End of namespace event
} // End of namespace Sigma

