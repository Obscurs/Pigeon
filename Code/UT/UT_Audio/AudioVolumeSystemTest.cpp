#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Audio/AudioVolumeSingletonComponent.h"
#include "Pigeon/Audio/AudioVolumeSystem.h"
#include "Pigeon/Audio/SetAudioVolumeRequestOneFrameComponent.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"

namespace CatchTestsetFail
{
	namespace
	{
		pg::EngineConfigSingletonComponent MakeConfig(float master, float sound, float music)
		{
			pg::EngineConfigSingletonComponent config;
			config.m_MasterVolume = master;
			config.m_SoundVolume = sound;
			config.m_MusicVolume = music;
			return config;
		}
	}

	// Happy path: with the engine config present, the system creates the volume singleton seeded from
	// the config's master/sound/music volumes.
	TEST_CASE("Audio.AudioVolumeSystem::CreatesVolumeFromConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioVolumeSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(0.5f, 0.8f, 0.3f));

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::AudioVolumeSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::AudioVolumeSingletonComponent& vol = view.get<pg::AudioVolumeSingletonComponent>(view.front());
		CHECK(vol.m_MasterVolume == Approx(0.5f));
		CHECK(vol.m_SoundVolume == Approx(0.8f));
		CHECK(vol.m_MusicVolume == Approx(0.3f));
	}

	// Guard: with no engine config the system creates nothing.
	TEST_CASE("Audio.AudioVolumeSystem::NoConfigCreatesNothing")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioVolumeSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::AudioVolumeSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// Guard: the singleton is created once and not duplicated on later frames.
	TEST_CASE("Audio.AudioVolumeSystem::DoesNotDuplicateVolume")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioVolumeSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(1.0f, 1.0f, 1.0f));

		world.Update(pg::Timestep(0));
		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::AudioVolumeSingletonComponent>();
		CHECK(view.size() == 1);
	}

	// Happy path: a volume request updates the live volumes in place.
	TEST_CASE("Audio.AudioVolumeSystem::AppliesVolumeRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::AudioVolumeSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(1.0f, 1.0f, 1.0f));

		world.Update(pg::Timestep(0)); // creates the volume singleton from config

		pg::SetAudioVolumeRequestOneFrameComponent request;
		request.m_MasterVolume = 0.2f;
		request.m_SoundVolume = 0.4f;
		request.m_MusicVolume = 0.6f;
		pg::ecs::Entity reqEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SetAudioVolumeRequestOneFrameComponent>(reqEnt, request);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::AudioVolumeSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::AudioVolumeSingletonComponent& vol = view.get<pg::AudioVolumeSingletonComponent>(view.front());
		CHECK(vol.m_MasterVolume == Approx(0.2f));
		CHECK(vol.m_SoundVolume == Approx(0.4f));
		CHECK(vol.m_MusicVolume == Approx(0.6f));
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Audio.AudioVolumeSystem::DeclareAccessIsCorrect")
	{
		pg::AudioVolumeSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::SetAudioVolumeRequestOneFrameComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::AudioVolumeSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::AudioVolumeSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
