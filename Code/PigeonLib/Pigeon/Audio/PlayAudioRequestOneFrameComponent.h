#pragma once

#include "Pigeon/Audio/EAudioCategory.h"
#include "Pigeon/Core/UUID.h"

namespace pg
{
	struct PlayAudioRequestOneFrameComponent
	{
		PlayAudioRequestOneFrameComponent() = default;
		PlayAudioRequestOneFrameComponent(const PlayAudioRequestOneFrameComponent&) = default;

		pg::UUID m_ClipID;                                   // sound resource to play
		pg::UUID m_VoiceID;                                  // caller-assigned handle to control this voice
		pg::EAudioCategory m_Category = pg::EAudioCategory::eSound;
		bool m_Loop = false;
		float m_Volume = 1.0f;                               // per-voice base volume
	};
}
