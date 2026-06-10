#pragma once
#include <glm/glm.hpp>

namespace pg
{
	// App-emitted one-frame request to update a camera. The app reads input and the camera's current
	// zoom + position, computes the new absolute values, and emits this; the engine CameraSystem applies
	// m_ZoomLevel to the OrthographicCameraComponent, pans the camera to m_Position via a
	// CameraTransformRequest, and rebuilds the projection. Engine-typed (pg) so the engine CameraSystem
	// can read an app-origin request.
	struct SetCameraRequestOneFrameComponent
	{
		SetCameraRequestOneFrameComponent() = default;
		SetCameraRequestOneFrameComponent(const SetCameraRequestOneFrameComponent&) = default;

		float m_ZoomLevel = 1.0f;
		glm::vec3 m_Position{ 0.f, 0.f, 0.f };
	};
}
