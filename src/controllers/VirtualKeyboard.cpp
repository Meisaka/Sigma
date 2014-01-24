#include "controllers/VirtualKeyboard.h"

#include "OS.h"

#include <iostream>

namespace Sigma {
	namespace event {
		namespace handler {

			VirtualKeyboard::VirtualKeyboard () : hasFocus(false), gkeyboard(nullptr) {
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
			}
			
			void VirtualKeyboard::KeyStateChange(const unsigned int key, const KEY_STATE state) {
				if (key == GLFW_KEY_F3 && state != KEY_STATE::KS_DOWN) { // F3 togles the keyboard focus
					this->hasFocus ^= 1;
					return;
				}

				if (this->gkeyboard != nullptr && hasFocus) {
					auto keycode = vm::aux::GLFWKeyToTR3200(key);
					bool kdown = state == KEY_STATE::KS_DOWN;

					this->gkeyboard->PushKeyEvent(kdown, keycode);
				}
			}

			void VirtualKeyboard::LostKeyboardFocus() {
			}

		} // End of namespace handler
	} // End of namespace event
} // End of namespace Sigma

