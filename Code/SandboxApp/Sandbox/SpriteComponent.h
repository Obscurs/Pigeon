#pragma once
#include "Pigeon/Core/UUID.h"

#include <glm/glm.hpp>

namespace sbx
{
	// A world-space sub-texture sprite. m_TexCoordsRect selects a rectangular region of the texture
	// in normalised UV space (x,y = min corner, z,w = max corner, each in [0,1]); the renderer uses
	// these values as raw UVs. Rendered via DrawSpriteInFrameEvent.
	struct SpriteComponent
	{
		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;

		glm::mat4 m_Transform{ 1.f };
		glm::vec4 m_TexCoordsRect{ 0.f, 0.f, 1.f, 1.f };
		glm::vec3 m_Origin{ 0.f, 0.f, 0.f };
		pg::UUID m_TextureID;
	};
}
