#pragma once
#include "Pigeon/Core/UUID.h"

#include <glm/glm.hpp>
#include <string>

namespace sbx
{
	// Visual data for a world-space text label. Placement comes from the transform components;
	// TextRenderSystem reads the resolved WorldTransform, resolves m_FontID against the resource map,
	// and emits a DrawStringInFrameEvent. The tagged readout label has its m_Text rewritten each frame
	// by InputReadoutSystem.
	struct LabelComponent
	{
		LabelComponent() = default;
		LabelComponent(const LabelComponent&) = default;

		std::string m_Text;
		pg::UUID m_FontID;
		glm::vec4 m_Color{ 1.f, 1.f, 1.f, 1.f };
		float m_Kerning{ 0.f };
		float m_Linespacing{ 0.f };
	};
}
