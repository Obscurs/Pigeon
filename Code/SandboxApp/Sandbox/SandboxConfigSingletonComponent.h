#pragma once
#include "Pigeon/Core/UUID.h"

namespace sbx
{
	// Resolved UUIDs read from Assets/App/Config.json. Every system in the showcase
	// looks elements, textures and fonts up through this single config so the JSON
	// assets stay the only place real UUIDs are written.
	struct SandboxConfigSingletonComponent
	{
		SandboxConfigSingletonComponent() {};
		SandboxConfigSingletonComponent(const SandboxConfigSingletonComponent&) = default;

		// Fonts (world-space text rendering).
		pg::UUID m_DefaultFontID;
		pg::UUID m_BoldFontID;

		// Textures (world-space draws).
		pg::UUID m_SpriteTextureID;       // sub-texture sprite demo
		pg::UUID m_TexturedQuadTextureID; // textured quad demo

		// UI layout to load on startup.
		pg::UUID m_MainLayoutID;

		// UI element ids the interaction systems target.
		pg::UUID m_ToggleButtonID; // click toggles the panel; hover swaps its image
		pg::UUID m_TogglePanelID;  // panel shown/hidden by the toggle button
		pg::UUID m_StatusTextID;   // live status text element
		pg::UUID m_CloseButtonID;  // dismiss button
		pg::UUID m_CloseTargetID;  // element destroyed by the dismiss button

		// Button textures swapped on interaction (default / hovered / pressed).
		pg::UUID m_ButtonImageID;
		pg::UUID m_ButtonHoverImageID;
		pg::UUID m_ButtonPressedImageID;

		// Audio clips for the sound demo.
		pg::UUID m_SampleSoundID; // one-shot sound effect
		pg::UUID m_SampleMusicID; // looping music track
	};
}
