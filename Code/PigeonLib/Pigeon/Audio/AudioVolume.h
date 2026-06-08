#pragma once

#include "Pigeon/Audio/EAudioCategory.h"

namespace pg
{
	// Effective linear gain for a voice: master scales everything, the per-category volume scales by
	// kind, and the base volume is the per-voice value requested at play time. Defined once here so
	// every audio backend mixes identically.
	inline float ComputeVoiceVolume(pg::EAudioCategory category, float master, float sound, float music, float base)
	{
		const float categoryVolume = (category == pg::EAudioCategory::eMusic) ? music : sound;
		return master * categoryVolume * base;
	}
}
