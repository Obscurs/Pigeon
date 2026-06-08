#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/ConfigLoaderSystem.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no EngineConfigSingletonComponent ->
	// system creates one via deferred add. Visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::CreatesEngineConfigOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		auto viewBefore = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with EngineConfigSingletonComponent already present ->
	// system does nothing (no duplicate created)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::DoesNotDuplicateWhenConfigExists")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		// First frame: the system itself creates the EngineConfigSingletonComponent
		// (the test must not pre-create a component the system adds).
		world.Update(pg::Timestep(0));

		// Second frame: the config already exists, so the guard fires and no duplicate is added.
		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded config has the three required UUID fields populated
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::LoadedConfigHasNonNullUUIDs")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::EngineConfigSingletonComponent& cfg =
			view.get<pg::EngineConfigSingletonComponent>(view.front());

		CHECK(!cfg.m_DefaultQuadShaderID.IsNull());
		CHECK(!cfg.m_DefaultTextShaderID.IsNull());
		CHECK(!cfg.m_DefaultFontID.IsNull());
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded config exposes the audio volumes within the valid [0,1] range
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::LoadedConfigHasAudioVolumes")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::EngineConfigSingletonComponent& cfg =
			view.get<pg::EngineConfigSingletonComponent>(view.front());

		CHECK(cfg.m_MasterVolume >= 0.0f);
		CHECK(cfg.m_MasterVolume <= 1.0f);
		CHECK(cfg.m_SoundVolume >= 0.0f);
		CHECK(cfg.m_SoundVolume <= 1.0f);
		CHECK(cfg.m_MusicVolume >= 0.0f);
		CHECK(cfg.m_MusicVolume <= 1.0f);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::DeclareAccessIsCorrect")
	{
		pg::ConfigLoaderSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
