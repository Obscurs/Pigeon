#pragma once

#include "Pigeon/Audio/AudioDevice.h"
#include "Pigeon/Core/Core.h"

namespace pg
{
	struct AudioDeviceSingletonComponent
	{
		AudioDeviceSingletonComponent() = default;
		AudioDeviceSingletonComponent(const AudioDeviceSingletonComponent&) = default;

		pg::S_Ptr<pg::AudioDevice> m_Device;
	};
}
