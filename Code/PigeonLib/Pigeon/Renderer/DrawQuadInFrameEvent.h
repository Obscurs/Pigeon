#pragma once
#include "Pigeon/Core/UUID.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pig
{
	struct DrawQuadInFrameEvent
	{
		DrawQuadInFrameEvent() {};
		DrawQuadInFrameEvent(const DrawQuadInFrameEvent&) = default;

		glm::mat4 m_Transform;
		glm::vec3 m_Color { 0.f, 0.f, 0.f };
		glm::vec3 m_Origin { 0.f, 0.f, 0.f };
		pig::UUID m_TextureID;
	};
}