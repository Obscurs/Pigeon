#pragma once

#include "Pigeon/Renderer/OrthographicCamera.h"
#include "Pigeon/Core/Timestep.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Events/MouseEvent.h"

namespace pig 
{
	class OrthographicCameraController
	{
	public:
		struct Data
		{
			float m_AspectRatio = 1.0f;
			float m_ZoomLevel = 1.0f;
			OrthographicCamera m_Camera = OrthographicCamera(0, 0, 0, 0);

			bool m_Rotation;

			glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
			float m_CameraRotation = 0.0f;
			float m_CameraTranslationSpeed = 5.0f;
			float m_CameraRotationSpeed = 180.0f;
			bool m_ReactsToInput = false;
		};

		OrthographicCameraController(bool reactsToInput, float aspectRatio, float z, bool rotation = false);

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void OnUpdate(const Timestep& ts);
		bool OnEvent(const Event& e);

		const OrthographicCamera& GetCamera() const { return m_Data.m_Camera; }

		float GetZoomLevel() const { return m_Data.m_ZoomLevel; }
		void SetZoomLevel(float level) { m_Data.m_ZoomLevel = level; }

	private:
		bool OnMouseScrolled(const MouseScrolledEvent& e);
		bool OnWindowResized(const WindowResizeEvent& e);
	private:
		Data m_Data;
	};
}