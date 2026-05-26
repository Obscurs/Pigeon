#pragma once
#include "Pigeon/Core/Core.h"
#include "Pigeon/Renderer/Sprite.h"

namespace pig
{
	struct DrawSpriteInFrameEvent
	{
		DrawSpriteInFrameEvent(const pig::Sprite& sprite) : m_Sprite(sprite) {};
		DrawSpriteInFrameEvent(const DrawSpriteInFrameEvent&) = default;

		pig::Sprite m_Sprite;
	};
}
