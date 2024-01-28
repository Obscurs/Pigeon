#pragma once

#include <glm/glm.hpp>

namespace pig 
{
	class OrthographicCamera
	{
	public:
		struct Data
		{
			glm::mat4 m_ProjectionMatrix;
			glm::mat4 m_ViewMatrix;
			glm::mat4 m_ViewProjectionMatrix;

			glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
			float m_Rotation = 0.0f;
		};

		OrthographicCamera(float left, float right, float bottom, float top);

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void SetProjection(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() const { return m_Data.m_Position; }
		void SetPosition(const glm::vec3& position) { m_Data.m_Position = position; RecalculateViewMatrix(); }

		float GetRotation() const { return m_Data.m_Rotation; }
		void SetRotation(float rotation) { m_Data.m_Rotation = rotation; RecalculateViewMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return m_Data.m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_Data.m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_Data.m_ViewProjectionMatrix; }

	private:
		void RecalculateViewMatrix();

		Data m_Data;
	};
}