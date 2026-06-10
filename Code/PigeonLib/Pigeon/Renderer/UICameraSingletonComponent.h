#pragma once
#include "Pigeon/Renderer/OrthographicCamera.h"

namespace pg
{
	// Screen-space orthographic camera the renderer uses for the dedicated UI pass. Owned by
	// the UI module (UIRenderSystem), read by Renderer2DSystem. The projection spans the live logical UI
	// canvas with a y-down convention (top-left origin), keeping UI independent of the gameplay camera's
	// pan/zoom. This is a pg:: (renderer) type so the renderer never depends on pg::ui types.
	struct UICameraSingletonComponent
	{
		UICameraSingletonComponent() {};
		UICameraSingletonComponent(const UICameraSingletonComponent&) = default;

		// Defaults to the 1920x1080 reference canvas; UIRenderSystem updates it to the live logical canvas.
		pg::OrthographicCamera m_Camera = pg::OrthographicCamera(0.f, 1920.f, 1080.f, 0.f);
	};
}
