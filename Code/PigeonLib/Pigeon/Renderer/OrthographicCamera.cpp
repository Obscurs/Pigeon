#include "pch.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

pig::OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
{
	m_Data.m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
	m_Data.m_ViewMatrix = glm::mat4(1.0f);
	m_Data.m_ViewProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
}

void pig::OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
{
	m_Data.m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
	m_Data.m_ViewProjectionMatrix = m_Data.m_ProjectionMatrix * m_Data.m_ViewMatrix;
}

void pig::OrthographicCamera::RecalculateViewMatrix()
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Data.m_Position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(m_Data.m_Rotation), glm::vec3(0, 0, 1));

	m_Data.m_ViewMatrix = glm::inverse(transform);
	m_Data.m_ViewProjectionMatrix = m_Data.m_ProjectionMatrix * m_Data.m_ViewMatrix;
}
