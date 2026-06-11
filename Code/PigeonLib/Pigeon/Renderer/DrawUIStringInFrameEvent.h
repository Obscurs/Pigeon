#pragma once
#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pg
{
	struct DrawUIStringInFrameEvent
	{
		DrawUIStringInFrameEvent() {};
		DrawUIStringInFrameEvent(const DrawUIStringInFrameEvent&) = default;

		pg::UUID m_FontID;
		std::string m_String = "";
		glm::mat4 m_Transform;
		glm::vec4 m_Color{ 0.f, 0.f, 0.f, 0.f };
		float m_Kerning = 0.f;
		float m_Linespacing = 0.f;

		// Typewriter reveal: -1 draws the whole string; N draws only glyphs whose source index is < N
		// (spaces/newlines still consume an index). Layout is computed on the full string so it never
		// reflows as the count advances.
		int m_VisibleChars = -1;

		// Scissor rect in window pixels (x, y, width, height) the draw is masked to; set by UIRenderSystem
		// to the element's effective clip rect, or the full window when unclipped.
		glm::vec4 m_ClipRect { 0.f, 0.f, 0.f, 0.f };
	};
}