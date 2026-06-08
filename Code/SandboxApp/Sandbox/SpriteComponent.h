#pragma once
#include "Pigeon/Core/UUID.h"

#include <glm/glm.hpp>

namespace sbx
{
	// Visual data for a world-space sub-texture sprite. Placement comes from the transform components;
	// SpriteRenderSystem reads the resolved WorldTransform. m_TexCoordsRect selects a rectangular region
	// of the texture in normalised UV space (x,y = min corner, z,w = max corner, each in [0,1]); the
	// renderer uses these values as raw UVs.
	struct SpriteComponent
	{
		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;

		glm::vec4 m_TexCoordsRect{ 0.f, 0.f, 1.f, 1.f };
		pg::UUID m_TextureID;
	};
}
