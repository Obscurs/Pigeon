#pragma once
#include "Pigeon/Core/Core.h"
#include "Pigeon/Renderer/Font.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pg
{
	struct DrawStringInFrameEvent
	{
		DrawStringInFrameEvent() {};
		DrawStringInFrameEvent(const DrawStringInFrameEvent&) = default;

		pg::S_Ptr<pg::Font> m_Font;
		std::string m_String = "";
		glm::mat4 m_Transform;
		glm::vec4 m_Color{ 0.f, 0.f, 0.f, 0.f };
		float m_Kerning = 0.f;
		float m_Linespacing = 0.f;
		float m_SortKey = 0.f; // world draw order: lower draws behind
	};
}