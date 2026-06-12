#pragma once
#include "Pigeon/Renderer/PerspectiveCamera.h"

namespace pg
{
	// The 3D camera placed in the world by the app and read by Renderer3DSystem to project models into
	// the offscreen render target. Mirrors OrthographicCameraComponent for the 2D world.
	struct PerspectiveCameraComponent
	{
		PerspectiveCameraComponent() = default;
		PerspectiveCameraComponent(const PerspectiveCameraComponent&) = default;

		pg::PerspectiveCamera m_Camera{ 0.785398f, 1.f, 0.1f, 100.f };
	};
}
