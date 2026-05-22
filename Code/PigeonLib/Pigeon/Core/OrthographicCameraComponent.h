#pragma once
#include "Pigeon/Renderer/OrthographicCamera.h"

namespace pig
{
	struct OrthographicCameraComponent
	{
		OrthographicCameraComponent() {};
		OrthographicCameraComponent(const OrthographicCameraComponent&) = default;

		float m_AspectRatio = 1.0f;
		float m_ZoomLevel = 1.0f;
		pig::OrthographicCamera m_Camera = pig::OrthographicCamera(0, 0, 0, 0);

		bool m_Rotation = false;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 5.0f;
		float m_CameraRotationSpeed = 180.0f;
		bool m_ReactsToInput = false;
	};
}
