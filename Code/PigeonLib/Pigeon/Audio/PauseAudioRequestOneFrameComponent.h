#pragma once

#include "Pigeon/Core/UUID.h"

namespace pg
{
	struct PauseAudioRequestOneFrameComponent
	{
		PauseAudioRequestOneFrameComponent() = default;
		PauseAudioRequestOneFrameComponent(const PauseAudioRequestOneFrameComponent&) = default;

		pg::UUID m_VoiceID;
	};
}
