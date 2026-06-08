#pragma once
#include "Pigeon/Renderer/OrthographicCamera.h"

namespace pg
{
	struct OrthographicCameraComponent
	{
		OrthographicCameraComponent() {};
		OrthographicCameraComponent(const OrthographicCameraComponent&) = default;

		float m_AspectRatio = 1.0f;
		float m_ZoomLevel = 1.0f;
		pg::OrthographicCamera m_Camera = pg::OrthographicCamera(-1.0f, 1.0f, -1.0f, 1.0f);

		// World position lives in the camera entity's PositionComponent; CameraSystem syncs m_Camera
		// from it and emits a CameraTransformRequest to pan it.
		float m_CameraTranslationSpeed = 5.0f;
		bool m_ReactsToInput = false;
	};
}
