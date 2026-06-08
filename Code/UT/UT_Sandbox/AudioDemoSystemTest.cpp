#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Audio/EAudioCategory.h"
#include "Pigeon/Audio/PauseAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/PlayAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/ResumeAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/StopAudioRequestOneFrameComponent.h"
#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/AudioDemoStateSingletonComponent.h"
#include "Sandbox/AudioDemoSystem.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"

namespace CatchTestsetFail
{
	namespace
	{
		struct Setup
		{
			pg::UUID m_SoundID;
			pg::UUID m_MusicID;
			pg::ecs::Entity m_InputEntity = pg::ecs::null;
		};

		Setup SeedConfigAndInput()
		{
			pg::ecs::Registry& reg = pg::World::GetRegistryDirect();

			Setup setup;
			setup.m_SoundID = pg::UUID::Generate();
			setup.m_MusicID = pg::UUID::Generate();

			pg::ecs::Entity cfgEnt = reg.create();
			sbx::SandboxConfigSingletonComponent& cfg = reg.emplace<sbx::SandboxConfigSingletonComponent>(cfgEnt);
			cfg.m_SampleSoundID = setup.m_SoundID;
			cfg.m_SampleMusicID = setup.m_MusicID;

			setup.m_InputEntity = reg.create();
			reg.emplace<pg::InputStateSingletonComponent>(setup.m_InputEntity);
			return setup;
		}

		pg::InputStateSingletonComponent& Input(pg::ecs::Entity inputEntity)
		{
			return pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(inputEntity);
		}
	}

	// Happy path: first frame creates the demo state singleton with a stable music voice handle.
	TEST_CASE("Sandbox.AudioDemoSystem::CreatesStateOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::AudioDemoSystem>());
		SeedConfigAndInput();

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<sbx::AudioDemoStateSingletonComponent>();
		REQUIRE(view.size() == 1);
		const sbx::AudioDemoStateSingletonComponent& state = view.get<sbx::AudioDemoStateSingletonComponent>(view.front());
		CHECK_FALSE(state.m_MusicVoiceID.IsNull());
		CHECK_FALSE(state.m_MusicPlaying);
	}

	// Happy path: pressing the sound key emits a one-shot sound play request.
	TEST_CASE("Sandbox.AudioDemoSystem::SoundKeyEmitsPlayRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::AudioDemoSystem>());
		Setup setup = SeedConfigAndInput();

		world.Update(pg::Timestep(0)); // create state

		Input(setup.m_InputEntity).m_KeysPressed[pg::PG_KEY_S] = 1;
		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::PlayAudioRequestOneFrameComponent>();
		REQUIRE(view.size() == 1);
		const pg::PlayAudioRequestOneFrameComponent& play = view.get<pg::PlayAudioRequestOneFrameComponent>(view.front());
		CHECK(play.m_ClipID == setup.m_SoundID);
		CHECK(play.m_Category == pg::EAudioCategory::eSound);
		CHECK_FALSE(play.m_Loop);
		CHECK_FALSE(play.m_VoiceID.IsNull());
	}

	// Happy path: pressing the music key starts looping music under the state's voice handle.
	TEST_CASE("Sandbox.AudioDemoSystem::MusicKeyStartsLoopingMusic")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::AudioDemoSystem>());
		Setup setup = SeedConfigAndInput();

		world.Update(pg::Timestep(0));

		Input(setup.m_InputEntity).m_KeysPressed[pg::PG_KEY_M] = 1;
		world.Update(pg::Timestep(0));

		auto stateView = pg::World::GetRegistryDirect().view<sbx::AudioDemoStateSingletonComponent>();
		REQUIRE(stateView.size() == 1);
		const sbx::AudioDemoStateSingletonComponent& state = stateView.get<sbx::AudioDemoStateSingletonComponent>(stateView.front());
		CHECK(state.m_MusicPlaying);

		auto playView = pg::World::GetRegistryDirect().view<pg::PlayAudioRequestOneFrameComponent>();
		REQUIRE(playView.size() == 1);
		const pg::PlayAudioRequestOneFrameComponent& play = playView.get<pg::PlayAudioRequestOneFrameComponent>(playView.front());
		CHECK(play.m_ClipID == setup.m_MusicID);
		CHECK(play.m_Category == pg::EAudioCategory::eMusic);
		CHECK(play.m_Loop);
		CHECK(play.m_VoiceID == state.m_MusicVoiceID);
	}

	// Happy path: pressing the music key again while playing stops the music.
	TEST_CASE("Sandbox.AudioDemoSystem::MusicKeyStopsWhenPlaying")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::AudioDemoSystem>());
		Setup setup = SeedConfigAndInput();

		world.Update(pg::Timestep(0)); // create state

		Input(setup.m_InputEntity).m_KeysPressed[pg::PG_KEY_M] = 1;
		world.Update(pg::Timestep(0)); // start music

		Input(setup.m_InputEntity).m_KeysPressed[pg::PG_KEY_M] = 2; // held, not a fresh press
		world.Update(pg::Timestep(0));

		Input(setup.m_InputEntity).m_KeysPressed[pg::PG_KEY_M] = 1; // fresh press again
		world.Update(pg::Timestep(0));

		auto stateView = pg::World::GetRegistryDirect().view<sbx::AudioDemoStateSingletonComponent>();
		REQUIRE(stateView.size() == 1);
		const sbx::AudioDemoStateSingletonComponent& state = stateView.get<sbx::AudioDemoStateSingletonComponent>(stateView.front());
		CHECK_FALSE(state.m_MusicPlaying);

		auto stopView = pg::World::GetRegistryDirect().view<pg::StopAudioRequestOneFrameComponent>();
		REQUIRE(stopView.size() == 1);
		CHECK(stopView.get<pg::StopAudioRequestOneFrameComponent>(stopView.front()).m_VoiceID == state.m_MusicVoiceID);
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Sandbox.AudioDemoSystem::DeclareAccessIsCorrect")
	{
		sbx::AudioDemoSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::AudioDemoStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::AudioDemoStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::PlayAudioRequestOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::StopAudioRequestOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::PauseAudioRequestOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ResumeAudioRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
