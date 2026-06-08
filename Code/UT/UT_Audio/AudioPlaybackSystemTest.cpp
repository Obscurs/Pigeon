#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Audio/AudioDevice.h"
#include "Pigeon/Audio/AudioDeviceSingletonComponent.h"
#include "Pigeon/Audio/AudioPlaybackSystem.h"
#include "Pigeon/Audio/AudioVolumeSingletonComponent.h"
#include "Pigeon/Audio/EAudioCategory.h"
#include "Pigeon/Audio/PauseAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/PlayAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/ResumeAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/SoundClip.h"
#include "Pigeon/Audio/StopAudioRequestOneFrameComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"

namespace CatchTestsetFail
{
	namespace
	{
		// Seeds a resource map holding one sound clip under clipId, plus a volume singleton at unit
		// volume. These are inputs the playback system reads but does not create.
		void SeedResourcesAndVolume(const pg::UUID& clipId)
		{
			pg::ecs::Registry& reg = pg::World::GetRegistryDirect();

			pg::ResourceMapSingletonComponent resources;
			resources.m_SoundMap[clipId] = pg::SoundClip::Create("dummy.wav");
			pg::ecs::Entity resEnt = reg.create();
			reg.emplace<pg::ResourceMapSingletonComponent>(resEnt, std::move(resources));

			pg::AudioVolumeSingletonComponent volume;
			pg::ecs::Entity volEnt = reg.create();
			reg.emplace<pg::AudioVolumeSingletonComponent>(volEnt, volume);
		}

		pg::S_Ptr<pg::AudioDevice> GetDevice()
		{
			auto view = pg::World::GetRegistryDirect().view<pg::AudioDeviceSingletonComponent>();
			REQUIRE(view.size() == 1);
			return view.get<pg::AudioDeviceSingletonComponent>(view.front()).m_Device;
		}

		pg::ecs::Entity EmplacePlay(const pg::UUID& clipId, const pg::UUID& voiceId, pg::EAudioCategory category, bool loop, float volume)
		{
			pg::PlayAudioRequestOneFrameComponent play;
			play.m_ClipID = clipId;
			play.m_VoiceID = voiceId;
			play.m_Category = category;
			play.m_Loop = loop;
			play.m_Volume = volume;
			pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
			pg::World::GetRegistryDirect().emplace<pg::PlayAudioRequestOneFrameComponent>(ent, play);
			return ent;
		}
	}

	// Happy path: the first frame lazily creates the audio device singleton holding a real device.
	TEST_CASE("Audio.AudioPlaybackSystem::CreatesDeviceOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioPlaybackSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::AudioDeviceSingletonComponent>();
		REQUIRE(view.size() == 1);
		CHECK(view.get<pg::AudioDeviceSingletonComponent>(view.front()).m_Device != nullptr);
	}

	// Happy path: a play request starts an active voice under the caller's handle.
	TEST_CASE("Audio.AudioPlaybackSystem::PlayRequestStartsVoice")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioPlaybackSystem>());
		world.Update(pg::Timestep(0)); // device created

		const pg::UUID clipId = pg::UUID::Generate();
		const pg::UUID voiceId = pg::UUID::Generate();
		SeedResourcesAndVolume(clipId);
		EmplacePlay(clipId, voiceId, pg::EAudioCategory::eSound, false, 1.0f);

		world.Update(pg::Timestep(0));

		pg::S_Ptr<pg::AudioDevice> device = GetDevice();
		CHECK(device->IsVoiceActive(voiceId));
		CHECK(device->GetActiveVoiceCount() == 1);
	}

	// Edge: multiple play requests in one frame start multiple simultaneous voices.
	TEST_CASE("Audio.AudioPlaybackSystem::MultipleVoicesPlaySimultaneously")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioPlaybackSystem>());
		world.Update(pg::Timestep(0));

		const pg::UUID clipId = pg::UUID::Generate();
		const pg::UUID voiceA = pg::UUID::Generate();
		const pg::UUID voiceB = pg::UUID::Generate();
		SeedResourcesAndVolume(clipId);
		EmplacePlay(clipId, voiceA, pg::EAudioCategory::eSound, false, 1.0f);
		EmplacePlay(clipId, voiceB, pg::EAudioCategory::eSound, false, 1.0f);

		world.Update(pg::Timestep(0));

		pg::S_Ptr<pg::AudioDevice> device = GetDevice();
		CHECK(device->IsVoiceActive(voiceA));
		CHECK(device->IsVoiceActive(voiceB));
		CHECK(device->GetActiveVoiceCount() == 2);
	}

	// Guard: a play request for a clip not in the resource map starts no voice.
	TEST_CASE("Audio.AudioPlaybackSystem::UnknownClipStartsNoVoice")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioPlaybackSystem>());
		world.Update(pg::Timestep(0));

		const pg::UUID clipId = pg::UUID::Generate();
		const pg::UUID voiceId = pg::UUID::Generate();
		SeedResourcesAndVolume(clipId);
		EmplacePlay(pg::UUID::Generate(), voiceId, pg::EAudioCategory::eSound, false, 1.0f); // different clip id

		world.Update(pg::Timestep(0));

		pg::S_Ptr<pg::AudioDevice> device = GetDevice();
		CHECK_FALSE(device->IsVoiceActive(voiceId));
		CHECK(device->GetActiveVoiceCount() == 0);
	}

	// Happy path: a stop request ends the matching voice.
	TEST_CASE("Audio.AudioPlaybackSystem::StopRequestStopsVoice")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioPlaybackSystem>());
		world.Update(pg::Timestep(0));

		const pg::UUID clipId = pg::UUID::Generate();
		const pg::UUID voiceId = pg::UUID::Generate();
		SeedResourcesAndVolume(clipId);
		pg::ecs::Entity playEnt = EmplacePlay(clipId, voiceId, pg::EAudioCategory::eMusic, true, 1.0f);

		world.Update(pg::Timestep(0));
		pg::World::GetRegistryDirect().destroy(playEnt); // do not replay the play next frame

		pg::S_Ptr<pg::AudioDevice> device = GetDevice();
		REQUIRE(device->IsVoiceActive(voiceId));

		pg::StopAudioRequestOneFrameComponent stop;
		stop.m_VoiceID = voiceId;
		pg::ecs::Entity stopEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::StopAudioRequestOneFrameComponent>(stopEnt, stop);

		world.Update(pg::Timestep(0));

		CHECK_FALSE(device->IsVoiceActive(voiceId));
	}

	// Happy path: pause then resume toggles the voice's paused state while keeping it active.
	TEST_CASE("Audio.AudioPlaybackSystem::PauseAndResumeVoice")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioPlaybackSystem>());
		world.Update(pg::Timestep(0));

		const pg::UUID clipId = pg::UUID::Generate();
		const pg::UUID voiceId = pg::UUID::Generate();
		SeedResourcesAndVolume(clipId);
		pg::ecs::Entity playEnt = EmplacePlay(clipId, voiceId, pg::EAudioCategory::eMusic, true, 1.0f);
		world.Update(pg::Timestep(0));
		pg::World::GetRegistryDirect().destroy(playEnt);

		pg::S_Ptr<pg::AudioDevice> device = GetDevice();

		pg::PauseAudioRequestOneFrameComponent pause;
		pause.m_VoiceID = voiceId;
		pg::ecs::Entity pauseEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::PauseAudioRequestOneFrameComponent>(pauseEnt, pause);
		world.Update(pg::Timestep(0));
		pg::World::GetRegistryDirect().destroy(pauseEnt);

		CHECK(device->IsVoiceActive(voiceId));
		CHECK(device->IsVoicePaused(voiceId));

		pg::ResumeAudioRequestOneFrameComponent resume;
		resume.m_VoiceID = voiceId;
		pg::ecs::Entity resumeEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResumeAudioRequestOneFrameComponent>(resumeEnt, resume);
		world.Update(pg::Timestep(0));

		CHECK(device->IsVoiceActive(voiceId));
		CHECK_FALSE(device->IsVoicePaused(voiceId));
	}

	// The effective voice volume reflects master * category * base from the volume singleton.
	TEST_CASE("Audio.AudioPlaybackSystem::VoiceVolumeReflectsCategoryVolumes")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioPlaybackSystem>());
		world.Update(pg::Timestep(0));

		const pg::UUID clipId = pg::UUID::Generate();
		const pg::UUID voiceId = pg::UUID::Generate();

		pg::ecs::Registry& reg = pg::World::GetRegistryDirect();
		pg::ResourceMapSingletonComponent resources;
		resources.m_SoundMap[clipId] = pg::SoundClip::Create("dummy.wav");
		pg::ecs::Entity resEnt = reg.create();
		reg.emplace<pg::ResourceMapSingletonComponent>(resEnt, std::move(resources));

		pg::AudioVolumeSingletonComponent volume;
		volume.m_MasterVolume = 0.5f;
		volume.m_SoundVolume = 0.4f;
		volume.m_MusicVolume = 1.0f;
		pg::ecs::Entity volEnt = reg.create();
		reg.emplace<pg::AudioVolumeSingletonComponent>(volEnt, volume);

		EmplacePlay(clipId, voiceId, pg::EAudioCategory::eSound, false, 0.5f);
		world.Update(pg::Timestep(0));

		pg::S_Ptr<pg::AudioDevice> device = GetDevice();
		CHECK(device->GetVoiceVolume(voiceId) == Approx(0.5f * 0.4f * 0.5f));
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Audio.AudioPlaybackSystem::DeclareAccessIsCorrect")
	{
		pg::AudioPlaybackSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::AudioVolumeSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::PlayAudioRequestOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::StopAudioRequestOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::PauseAudioRequestOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResumeAudioRequestOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::AudioDeviceSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
