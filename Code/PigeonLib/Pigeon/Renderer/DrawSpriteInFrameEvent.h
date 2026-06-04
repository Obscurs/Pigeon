#pragma once
#include "Pigeon/Core/Core.h"
#include "Pigeon/Renderer/Sprite.h"

namespace pg
{
	struct DrawSpriteInFrameEvent
	{
		DrawSpriteInFrameEvent(const pg::Sprite& sprite) : m_Sprite(sprite) {};
		DrawSpriteInFrameEvent(const DrawSpriteInFrameEvent&) = default;

		pg::Sprite m_Sprite;
	};
}
