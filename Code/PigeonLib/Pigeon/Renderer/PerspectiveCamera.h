#pragma once

#include <glm/glm.hpp>

namespace pg
{
	// The 3D camera: a perspective projection (vertical FOV, aspect, near/far) combined with a look-at
	// view (eye position, target, up). Left-handed with zero-to-one depth to match DirectX. The 2D
	// counterpart is OrthographicCamera; the app authors placement, Renderer3DSystem reads the matrix.
	class PerspectiveCamera
	{
	public:
		struct Data
		{
			glm::mat4 m_ProjectionMatrix{ 1.f };
			glm::mat4 m_ViewMatrix{ 1.f };
			glm::mat4 m_ViewProjectionMatrix{ 1.f };

			glm::vec3 m_Position{ 0.f, 0.f, -1.f };
			glm::vec3 m_Target{ 0.f, 0.f, 0.f };
			glm::vec3 m_Up{ 0.f, 1.f, 0.f };

			float m_FovY{ 0.785398f }; // 45 degrees in radians
			float m_Aspect{ 1.f };
			float m_NearZ{ 0.1f };
			float m_FarZ{ 100.f };
		};

		PerspectiveCamera(float fovYRadians, float aspect, float nearZ, float farZ);

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void SetProjection(float fovYRadians, float aspect, float nearZ, float farZ);
		void SetView(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

		const glm::vec3& GetPosition() const { return m_Data.m_Position; }
		const glm::vec3& GetTarget() const { return m_Data.m_Target; }
		float GetAspect() const { return m_Data.m_Aspect; }

		const glm::mat4& GetProjectionMatrix() const { return m_Data.m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_Data.m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_Data.m_ViewProjectionMatrix; }

	private:
		void RecalculateViewProjection();

		Data m_Data;
	};
}
