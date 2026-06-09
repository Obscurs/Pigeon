#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#include "Pigeon/Core/EWindowMode.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/SetWindowResolutionRequestOneFrameComponent.h"
#include "Pigeon/Core/WindowConfigSingletonComponent.h"
#include "Pigeon/Core/WindowConfigSystem.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"

namespace CatchTestsetFail
{
	namespace
	{
		pg::EngineConfigSingletonComponent MakeWindowConfig(unsigned int width, unsigned int height,
			pg::EWindowMode mode, const std::string& savedataPath)
		{
			pg::EngineConfigSingletonComponent config;
			config.m_WindowWidth = width;
			config.m_WindowHeight = height;
			config.m_WindowMode = mode;
			config.m_SavedataPath = savedataPath;
			return config;
		}

		json ReadJson(const std::string& path)
		{
			std::ifstream file(path);
			REQUIRE(file.is_open());
			std::ostringstream ss;
			ss << file.rdbuf();
			return json::parse(ss.str());
		}

		void WriteText(const std::string& path, const std::string& content)
		{
			std::ofstream file(path);
			REQUIRE(file.is_open());
			file << content;
		}
	}

	// Happy path: with the engine config present, the system seeds the window-config singleton from the
	// config's window width/height/mode. The Testing build has no Application, so the window apply is skipped.
	TEST_CASE("Core.WindowConfigSystem::CreatesWindowConfigFromEngineConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::WindowConfigSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(
			configEnt, MakeWindowConfig(1600, 900, pg::EWindowMode::eFullscreen, ""));

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::WindowConfigSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::WindowConfigSingletonComponent& wc = view.get<pg::WindowConfigSingletonComponent>(view.front());
		CHECK(wc.m_Width == 1600);
		CHECK(wc.m_Height == 900);
		CHECK(wc.m_Mode == pg::EWindowMode::eFullscreen);
	}

	// Guard: with no engine config the system creates nothing.
	TEST_CASE("Core.WindowConfigSystem::NoConfigCreatesNothing")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::WindowConfigSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::WindowConfigSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// Guard: the singleton is created once and not duplicated on later frames.
	TEST_CASE("Core.WindowConfigSystem::DoesNotDuplicateWindowConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::WindowConfigSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(
			configEnt, MakeWindowConfig(1280, 720, pg::EWindowMode::eWindowed, ""));

		world.Update(pg::Timestep(0));
		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::WindowConfigSingletonComponent>();
		CHECK(view.size() == 1);
	}

	// Happy path: a resolution request updates the live window-config singleton in place.
	TEST_CASE("Core.WindowConfigSystem::AppliesResolutionRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::WindowConfigSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(
			configEnt, MakeWindowConfig(1280, 720, pg::EWindowMode::eWindowed, ""));

		world.Update(pg::Timestep(0)); // seeds the singleton from config

		pg::SetWindowResolutionRequestOneFrameComponent request;
		request.m_Width = 1920;
		request.m_Height = 1080;
		request.m_Mode = pg::EWindowMode::eFullscreen;
		pg::ecs::Entity reqEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SetWindowResolutionRequestOneFrameComponent>(reqEnt, request);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::WindowConfigSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::WindowConfigSingletonComponent& wc = view.get<pg::WindowConfigSingletonComponent>(view.front());
		CHECK(wc.m_Width == 1920);
		CHECK(wc.m_Height == 1080);
		CHECK(wc.m_Mode == pg::EWindowMode::eFullscreen);
	}

	// Persistence: a resolution request rewrites the savedata Config.json with the new window values,
	// merging into the existing file so other override keys are preserved.
	TEST_CASE("Core.WindowConfigSystem::PersistsResolutionToSavedataConfig")
	{
		const std::string savedataDir = ".";
		const std::string savedataConfigPath = "./Config.json";

		// A pre-existing savedata override that must survive the rewrite.
		WriteText(savedataConfigPath, "{\n\t\"musicVolume\": 0.3\n}\n");

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::WindowConfigSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(
			configEnt, MakeWindowConfig(1280, 720, pg::EWindowMode::eWindowed, savedataDir));

		world.Update(pg::Timestep(0)); // seeds the singleton; must NOT write the file yet

		pg::SetWindowResolutionRequestOneFrameComponent request;
		request.m_Width = 1920;
		request.m_Height = 1080;
		request.m_Mode = pg::EWindowMode::eFullscreen;
		pg::ecs::Entity reqEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SetWindowResolutionRequestOneFrameComponent>(reqEnt, request);

		world.Update(pg::Timestep(0)); // applies + persists

		const json saved = ReadJson(savedataConfigPath);
		CHECK(saved.contains("windowWidth"));
		CHECK(saved["windowWidth"].get<unsigned int>() == 1920);
		CHECK(saved["windowHeight"].get<unsigned int>() == 1080);
		CHECK(saved["windowMode"].get<std::string>() == "fullscreen");
		// The unrelated override key is preserved.
		REQUIRE(saved.contains("musicVolume"));
		CHECK(saved["musicVolume"].get<float>() == Approx(0.3f));

		std::remove(savedataConfigPath.c_str());
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Core.WindowConfigSystem::DeclareAccessIsCorrect")
	{
		pg::WindowConfigSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::SetWindowResolutionRequestOneFrameComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::WindowConfigSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::WindowConfigSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
