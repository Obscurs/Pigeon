#pragma once

namespace pg
{
	// Per-entity request to update sprite animation playback, emitted by gameplay (app) systems and
	// applied by SpriteAnimationSystem. Like the other engine requests (camera, audio, window), it is an
	// engine (pg) type so the engine system can read it. m_SetRow switches the active animation row to
	// m_Row (left unchanged when false, so an idle character keeps facing its last direction); m_Playing
	// toggles frame advancement.
	struct SetSpriteAnimationRequestOneFrameComponent
	{
		SetSpriteAnimationRequestOneFrameComponent() = default;
		SetSpriteAnimationRequestOneFrameComponent(const SetSpriteAnimationRequestOneFrameComponent&) = default;

		bool m_SetRow = false;
		int m_Row = 0;
		bool m_Playing = true;
	};
}
