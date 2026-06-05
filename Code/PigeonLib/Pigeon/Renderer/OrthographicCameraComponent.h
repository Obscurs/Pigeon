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

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraTranslationSpeed = 5.0f;
		bool m_ReactsToInput = false;
	};
}
