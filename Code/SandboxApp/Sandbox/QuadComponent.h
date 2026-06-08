#pragma once
#include "Pigeon/Core/UUID.h"

#include <glm/glm.hpp>

namespace sbx
{
	// Visual data for a world-space quad. Placement comes from the transform components
	// (Position/Rotation/Scale/LocalBounds); QuadRenderSystem reads the resolved WorldTransform.
	// m_TextureID null => flat colour quad; set => textured quad.
	struct QuadComponent
	{
		QuadComponent() = default;
		QuadComponent(const QuadComponent&) = default;

		glm::vec3 m_Color{ 1.f, 1.f, 1.f };
		pg::UUID m_TextureID;
	};
}
