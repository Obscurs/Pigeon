#include "pch.h"
#include "OrthographicCameraController.h"

#include "Pigeon/Core/InputLayer.h"
#include "Pigeon/Core/KeyCodes.h"

pig::OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
{
	m_Data.m_AspectRatio = aspectRatio;
	m_Data.m_Camera = OrthographicCamera(-m_Data.m_AspectRatio * m_Data.m_ZoomLevel, m_Data.m_AspectRatio * m_Data.m_ZoomLevel, -m_Data.m_ZoomLevel, m_Data.m_ZoomLevel);
	m_Data.m_Rotation = rotation;
}

void pig::OrthographicCameraController::OnUpdate(const Timestep& ts)
{
	if (Input::IsKeyPressed(PG_KEY_A))
		m_Data.m_CameraPosition.x -= m_Data.m_CameraTranslationSpeed * ts.AsSeconds();
	else if (Input::IsKeyPressed(PG_KEY_D))
		m_Data.m_CameraPosition.x += m_Data.m_CameraTranslationSpeed * ts.AsSeconds();

	if (Input::IsKeyPressed(PG_KEY_W))
		m_Data.m_CameraPosition.y += m_Data.m_CameraTranslationSpeed * ts.AsSeconds();
	else if (Input::IsKeyPressed(PG_KEY_S))
		m_Data.m_CameraPosition.y -= m_Data.m_CameraTranslationSpeed * ts.AsSeconds();

	/*if (m_Data.m_Rotation)
	{
		if (Input::IsKeyPressed(PG_KEY_Q))
			m_Data.m_CameraRotation += m_Data.m_CameraRotationSpeed * ts;
		if (Input::IsKeyPressed(PG_KEY_E))
			m_Data.m_CameraRotation -= m_Data.m_CameraRotationSpeed * ts;

		m_Data.m_Camera.SetRotation(m_Data.m_CameraRotation);
	}*/

	m_Data.m_Camera.SetPosition(m_Data.m_CameraPosition);

	m_Data.m_CameraTranslationSpeed = m_Data.m_ZoomLevel;
}

bool pig::OrthographicCameraController::OnEvent(const Event& e)
{
	return
		pig::EventDispatcher::Dispatch<MouseScrolledEvent>(e, pig::BindEventFn<&OrthographicCameraController::OnMouseScrolled, OrthographicCameraController>(this)) ||
		pig::EventDispatcher::Dispatch<WindowResizeEvent>(e, pig::BindEventFn<&OrthographicCameraController::OnWindowResized, OrthographicCameraController>(this));
}

bool pig::OrthographicCameraController::OnMouseScrolled(const MouseScrolledEvent& e)
{
	m_Data.m_ZoomLevel -= e.GetYOffset() * 0.25f;
	m_Data.m_ZoomLevel = std::max(m_Data.m_ZoomLevel, 0.25f);
	m_Data.m_Camera.SetProjection(-m_Data.m_AspectRatio * m_Data.m_ZoomLevel, m_Data.m_AspectRatio * m_Data.m_ZoomLevel, -m_Data.m_ZoomLevel, m_Data.m_ZoomLevel);
	return false;
}

bool pig::OrthographicCameraController::OnWindowResized(const WindowResizeEvent& e)
{
	m_Data.m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
	m_Data.m_Camera.SetProjection(-m_Data.m_AspectRatio * m_Data.m_ZoomLevel, m_Data.m_AspectRatio * m_Data.m_ZoomLevel, -m_Data.m_ZoomLevel, m_Data.m_ZoomLevel);
	return false;
}