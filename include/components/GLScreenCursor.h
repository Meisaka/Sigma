#pragma once

#ifndef GL_SCREEN_CURSOR
#define GL_SCREEN_CURSOR

#include "GLScreenQuad.h"
#include "systems/MouseInputSystem.h"

namespace Sigma {
	namespace event {
		namespace handler {
class GLScreenCursor : public GLScreenQuad, public IMouseEventHandler {
public:
	SET_COMPONENT_TYPENAME("GLScreenCursor");
	GLScreenCursor(int entityID);
	virtual ~GLScreenCursor();

	virtual void InitializeBuffers();
	virtual void Render(glm::mediump_float *view, glm::mediump_float *proj);

	// Texture space 0.0 - 1.0
	void SetHotspot(float x, float y) { this->hotspotx=x; this->hotspoty=y; }

	virtual void MouseMove(float x, float y, float dx, float dy);
	virtual void MouseVisible(bool visible) { this->show = visible; }
	// Not used but required.
	virtual void MouseDown(Sigma::event::BUTTON btn, float x, float y);
	virtual void MouseUp(Sigma::event::BUTTON btn, float x, float y);
protected:
	float hotspotx, hotspoty;
	bool show;
};
		};
	};
};

#endif