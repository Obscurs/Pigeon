#pragma once
#include "Pigeon/Core/Core.h"
#include "Pigeon/Renderer/Sprite.h"

namespace pg
{
	struct DrawSpriteInFrameEvent
	{
		DrawSpriteInFrameEvent(const pg::Sprite& sprite) : m_Sprite(sprite) {};
		DrawSpriteInFrameEvent(const pg::Sprite& sprite, float sortKey) : m_Sprite(sprite), m_SortKey(sortKey) {};
		DrawSpriteInFrameEvent(const DrawSpriteInFrameEvent&) = default;

		pg::Sprite m_Sprite;
		float m_SortKey { 0.f }; // world draw order: lower draws behind
	};
}
