#pragma once
#include <catch2/catch.hpp>

#include "Pigeon/Audio/AudioVolume.h"
#include "Pigeon/Audio/EAudioCategory.h"

namespace CatchTestsetFail
{
	// ComputeVoiceVolume multiplies master * category volume * base, picking the sound volume for
	// eSound voices.
	TEST_CASE("Audio.AudioVolume::SoundCategoryUsesSoundVolume")
	{
		const float result = pg::ComputeVoiceVolume(pg::EAudioCategory::eSound, 0.5f, 0.8f, 0.2f, 0.5f);
		CHECK(result == Approx(0.5f * 0.8f * 0.5f));
	}

	// ComputeVoiceVolume picks the music volume for eMusic voices.
	TEST_CASE("Audio.AudioVolume::MusicCategoryUsesMusicVolume")
	{
		const float result = pg::ComputeVoiceVolume(pg::EAudioCategory::eMusic, 0.5f, 0.8f, 0.2f, 1.0f);
		CHECK(result == Approx(0.5f * 0.2f * 1.0f));
	}

	// Unit volumes leave the base volume unchanged.
	TEST_CASE("Audio.AudioVolume::UnitVolumesPassThrough")
	{
		const float result = pg::ComputeVoiceVolume(pg::EAudioCategory::eSound, 1.0f, 1.0f, 1.0f, 0.7f);
		CHECK(result == Approx(0.7f));
	}

} // namespace CatchTestsetFail
