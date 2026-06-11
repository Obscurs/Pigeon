#pragma once
#include "Pigeon/Renderer/SpriteSheet.h"

namespace pg
{
	// Drives a SpriteComponent's sub-texture through a sprite sheet over time. SpriteAnimationSystem is
	// the sole writer: each frame it advances m_Elapsed and steps m_Column across the active row's frames
	// (wrapping at m_FrameCount), then writes the resulting cell's UV rectangle into the SpriteComponent.
	// m_Row selects which sheet row (animation) plays; m_Playing == false freezes the sprite on its idle
	// frame. The app fills these fields in as configuration (sheet, frame count, speed, starting row); an
	// entity needs both this and a SpriteComponent to be animated.
	struct SpriteAnimationComponent
	{
		SpriteAnimationComponent() = default;
		SpriteAnimationComponent(const SpriteAnimationComponent&) = default;

		pg::SpriteSheet m_Sheet;       // grid layout of the sprite's texture
		int m_Row = 0;                 // active animation (sheet row)
		int m_Column = 0;              // current frame within the active row
		int m_FrameCount = 1;          // number of leading columns cycled through as frames
		float m_FrameDuration = 0.1f;  // seconds each frame is shown (animation speed)
		float m_Elapsed = 0.f;         // time accumulated toward the next frame
		bool m_Playing = true;         // false freezes on the idle frame
	};
}
