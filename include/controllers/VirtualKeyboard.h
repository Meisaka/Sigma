
#pragma once
#ifndef _VIRTUAL_KEYBOARD_H_
#define _VIRTUAL_KEYBOARD_H_ 1

#include "vm/include/VM.hpp"

#include "systems/KeyboardInputSystem.h"
#include "components/ALSound.h"
#include "Sigma.h"

namespace Sigma {
	namespace event {
		namespace handler {
			class VirtualKeyboard : public IKeyboardEventHandler {
			public:
				DLL_EXPORT VirtualKeyboard ();

				void SetKeyboardDevice(vm::keyboard::GKeyboard* gkeyboard) {
					this->gkeyboard = gkeyboard;
				}
				
				/**
				 * \brief Triggered whenever a key state change event happens
				 *
				 * This method adjusts the view mover according to various key state changes.
				 * \param key The key for which the state change is happening
				 * \param state The new state of the key (KS_UP or KS_DOWN)
				 * \return void
				 */
				DLL_EXPORT void KeyStateChange(const unsigned int key, const KEY_STATE state);
				DLL_EXPORT void KeyStateChange(const unsigned int key, const KEY_STATE state, const KEY_STATE laststate);

				/**
				 * \brief Called when focus for this controller has been lost.
				 *
				 * \return void
				 */
				void LostKeyboardFocus();

				ALSound* actionsound;
				ALSound* reactionsound;
				ALSound* spactionsound;
				vm::VirtualComputer<vm::cpu::TR3200>* vcvm;
			private:
				bool hasFocus;
				vm::keyboard::GKeyboard* gkeyboard;
			};

		} // End of namespace handler
	} // End of namespace event
} // End of namespace Sigma


#endif // _VIRTUAL_KEYBOARD_H_

