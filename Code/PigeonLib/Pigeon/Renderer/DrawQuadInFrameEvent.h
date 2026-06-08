#pragma once
#include "Pigeon/Core/UUID.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pg
{
	struct DrawQuadInFrameEvent
	{
		DrawQuadInFrameEvent() {};
		DrawQuadInFrameEvent(const DrawQuadInFrameEvent&) = default;

		glm::mat4 m_Transform;
		glm::vec3 m_Color { 0.f, 0.f, 0.f };
		glm::vec3 m_Origin { 0.f, 0.f, 0.f };
		pg::UUID m_TextureID;
		float m_SortKey { 0.f }; // world draw order: lower draws behind
	};
}