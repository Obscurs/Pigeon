#include "pch.h"
#include "Pigeon/Renderer/PerspectiveCamera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

pg::PerspectiveCamera::PerspectiveCamera(float fovYRadians, float aspect, float nearZ, float farZ)
{
	m_Data.m_FovY = fovYRadians;
	m_Data.m_Aspect = aspect;
	m_Data.m_NearZ = nearZ;
	m_Data.m_FarZ = farZ;
	RecalculateViewProjection();
}

void pg::PerspectiveCamera::SetProjection(float fovYRadians, float aspect, float nearZ, float farZ)
{
	m_Data.m_FovY = fovYRadians;
	m_Data.m_Aspect = aspect;
	m_Data.m_NearZ = nearZ;
	m_Data.m_FarZ = farZ;
	RecalculateViewProjection();
}

void pg::PerspectiveCamera::SetView(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
	m_Data.m_Position = position;
	m_Data.m_Target = target;
	m_Data.m_Up = up;
	RecalculateViewProjection();
}

void pg::PerspectiveCamera::RecalculateViewProjection()
{
	// Left-handed, zero-to-one depth so the clip-space Z matches DirectX's [0,1] depth range; the
	// shader/upload matrix convention is identical to the 2D camera (the renderer multiplies the same
	// way), only the matrix values differ.
	m_Data.m_ProjectionMatrix = glm::perspectiveLH_ZO(m_Data.m_FovY, m_Data.m_Aspect, m_Data.m_NearZ, m_Data.m_FarZ);
	m_Data.m_ViewMatrix = glm::lookAtLH(m_Data.m_Position, m_Data.m_Target, m_Data.m_Up);
	m_Data.m_ViewProjectionMatrix = m_Data.m_ProjectionMatrix * m_Data.m_ViewMatrix;
}
