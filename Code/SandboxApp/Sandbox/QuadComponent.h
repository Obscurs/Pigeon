#pragma once
#include "Pigeon/Core/UUID.h"

#include <glm/glm.hpp>

namespace sbx
{
	// A world-space quad to render. m_TextureID null => flat colour quad; set => textured quad.
	// The z component of m_Transform's translation selects the render layer (Renderer2D layering).
	struct QuadComponent
	{
		QuadComponent() = default;
		QuadComponent(const QuadComponent&) = default;

		glm::mat4 m_Transform{ 1.f };
		glm::vec3 m_Color{ 1.f, 1.f, 1.f };
		glm::vec3 m_Origin{ 0.f, 0.f, 0.f };
		pg::UUID m_TextureID;
	};
}
