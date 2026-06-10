#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/ConfigLoaderSystem.h"
#include "Pigeon/Core/EWindowMode.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"

namespace CatchTestsetFail
{
	namespace
	{
		// Reads a JSON file relative to the test working directory (bin/<Config>).
		json LoadJsonFixture(const std::string& path)
		{
			std::ifstream file(path);
			REQUIRE(file.is_open());
			std::ostringstream ss;
			ss << file.rdbuf();
			return json::parse(ss.str());
		}
	}

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
	// Happy path: loaded config exposes a positive default window resolution and a
	// valid display mode (windowed or fullscreen).
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::LoadedConfigHasWindowResolution")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::EngineConfigSingletonComponent& cfg =
			view.get<pg::EngineConfigSingletonComponent>(view.front());

		CHECK(cfg.m_WindowWidth > 0);
		CHECK(cfg.m_WindowHeight > 0);
		CHECK((cfg.m_WindowMode == pg::EWindowMode::eWindowed || cfg.m_WindowMode == pg::EWindowMode::eFullscreen));
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded config exposes a positive UI reference (canvas) resolution
	// and a match factor within the valid [0,1] range (defaults when keys absent).
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::LoadedConfigHasUICanvasParams")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::EngineConfigSingletonComponent& cfg =
			view.get<pg::EngineConfigSingletonComponent>(view.front());

		CHECK(cfg.m_UIReferenceWidth > 0.f);
		CHECK(cfg.m_UIReferenceHeight > 0.f);
		CHECK(cfg.m_UIMatchFactor >= 0.f);
		CHECK(cfg.m_UIMatchFactor <= 1.f);
	}

	// ---------------------------------------------------------------------------
	// Savedata override: the savedata window width wins over the engine config
	// value, mirroring the audio-volume override semantics.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::SavedataConfigOverridesWindowResolution")
	{
		const json engineJson = LoadJsonFixture("Assets/Engine/Config.json");
		REQUIRE(engineJson.contains("savedataPath"));

		const std::string savedataConfigPath =
			engineJson["savedataPath"].get<std::string>() + "/Config.json";
		const json savedataJson = LoadJsonFixture(savedataConfigPath);

		REQUIRE(engineJson.contains("windowWidth"));
		REQUIRE(savedataJson.contains("windowWidth"));
		const unsigned int savedataWidth = savedataJson["windowWidth"].get<unsigned int>();
		// The override is only meaningful if the two values actually differ.
		REQUIRE(engineJson["windowWidth"].get<unsigned int>() != savedataWidth);

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::EngineConfigSingletonComponent& cfg =
			view.get<pg::EngineConfigSingletonComponent>(view.front());

		CHECK(cfg.m_WindowWidth == savedataWidth);
	}

	// ---------------------------------------------------------------------------
	// The engine config records the savedata path so the runtime can persist back to it.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::LoadedConfigRecordsSavedataPath")
	{
		const json engineJson = LoadJsonFixture("Assets/Engine/Config.json");
		REQUIRE(engineJson.contains("savedataPath"));
		const std::string expectedPath = engineJson["savedataPath"].get<std::string>();

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::EngineConfigSingletonComponent& cfg =
			view.get<pg::EngineConfigSingletonComponent>(view.front());

		CHECK(cfg.m_SavedataPath == expectedPath);
	}

	// ---------------------------------------------------------------------------
	// Savedata override: a value present in the savedata config wins over the
	// engine config value (override semantics).
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::SavedataConfigOverridesEngineValue")
	{
		const json engineJson = LoadJsonFixture("Assets/Engine/Config.json");
		REQUIRE(engineJson.contains("savedataPath"));

		const std::string savedataConfigPath =
			engineJson["savedataPath"].get<std::string>() + "/Config.json";
		const json savedataJson = LoadJsonFixture(savedataConfigPath);

		REQUIRE(engineJson.contains("musicVolume"));
		REQUIRE(savedataJson.contains("musicVolume"));
		const float engineMusic = engineJson["musicVolume"].get<float>();
		const float savedataMusic = savedataJson["musicVolume"].get<float>();
		// The override is only meaningful if the two values actually differ.
		REQUIRE(engineMusic != Approx(savedataMusic));

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::EngineConfigSingletonComponent& cfg =
			view.get<pg::EngineConfigSingletonComponent>(view.front());

		CHECK(cfg.m_MusicVolume == Approx(savedataMusic));
	}

	// ---------------------------------------------------------------------------
	// Savedata override: a key absent from the savedata config keeps the engine
	// config value (partial override / fallback).
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::EngineValueKeptWhenSavedataOmitsKey")
	{
		const json engineJson = LoadJsonFixture("Assets/Engine/Config.json");
		REQUIRE(engineJson.contains("savedataPath"));

		const std::string savedataConfigPath =
			engineJson["savedataPath"].get<std::string>() + "/Config.json";
		const json savedataJson = LoadJsonFixture(savedataConfigPath);

		REQUIRE(engineJson.contains("masterVolume"));
		// This test relies on the savedata fixture NOT overriding masterVolume.
		REQUIRE(!savedataJson.contains("masterVolume"));
		const float engineMaster = engineJson["masterVolume"].get<float>();

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::EngineConfigSingletonComponent& cfg =
			view.get<pg::EngineConfigSingletonComponent>(view.front());

		CHECK(cfg.m_MasterVolume == Approx(engineMaster));
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
