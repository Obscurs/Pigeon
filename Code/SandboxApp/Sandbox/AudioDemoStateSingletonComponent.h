#pragma once
#include "Pigeon/Core/UUID.h"

namespace sbx
{
	// Tracks the demo's single looping music voice and its play/pause state so the keyboard toggles act
	// on the same voice handle across frames.
	struct AudioDemoStateSingletonComponent
	{
		AudioDemoStateSingletonComponent() = default;
		AudioDemoStateSingletonComponent(const AudioDemoStateSingletonComponent&) = default;

		pg::UUID m_MusicVoiceID;
		bool m_MusicPlaying = false;
		bool m_MusicPaused = false;
	};
}
