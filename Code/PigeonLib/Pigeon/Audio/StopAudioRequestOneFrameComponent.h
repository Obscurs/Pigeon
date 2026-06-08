#pragma once

#include "Pigeon/Core/UUID.h"

namespace pg
{
	struct StopAudioRequestOneFrameComponent
	{
		StopAudioRequestOneFrameComponent() = default;
		StopAudioRequestOneFrameComponent(const StopAudioRequestOneFrameComponent&) = default;

		pg::UUID m_VoiceID;
	};
}
