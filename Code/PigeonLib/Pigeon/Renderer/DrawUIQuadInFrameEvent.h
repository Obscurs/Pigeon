#pragma once
#include "Pigeon/Core/UUID.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pg
{
	struct DrawUIQuadInFrameEvent
	{
		DrawUIQuadInFrameEvent() {};
		DrawUIQuadInFrameEvent(const DrawUIQuadInFrameEvent&) = default;

		glm::mat4 m_Transform;
		glm::vec3 m_Color { 0.f, 0.f, 0.f };
		glm::vec3 m_Origin { 0.f, 0.f, 0.f };
		pg::UUID m_TextureID;

		// Texture UV sub-rect (u0, v0, u1, v1) the quad samples; defaults to the full texture. Nine-slice
		// emits sub-quads with per-cell UVs
		glm::vec4 m_TexCoords { 0.f, 0.f, 1.f, 1.f };

		// Scissor rect in window pixels (x, y, width, height) the draw is masked to; set by UIRenderSystem
		// to the element's effective clip rect, or the full window when unclipped
		glm::vec4 m_ClipRect { 0.f, 0.f, 0.f, 0.f };
	};
}