#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/ConfigLoaderSystem.h>
#include <Pigeon/Core/EngineConfigSingletonComponent.h>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no EngineConfigSingletonComponent ->
	// system creates one via deferred add. Visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::CreatesEngineConfigOnFirstFrame")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ConfigLoaderSystem>());

		auto viewBefore = pig::World::GetRegistryDirect().view<pig::EngineConfigSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pig::Timestep(0));

		auto viewAfter = pig::World::GetRegistryDirect().view<pig::EngineConfigSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with EngineConfigSingletonComponent already present ->
	// system does nothing (no duplicate created)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::DoesNotDuplicateWhenConfigExists")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ConfigLoaderSystem>());

		// Pre-seed an EngineConfigSingletonComponent so the system guard fires.
		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::EngineConfigSingletonComponent>(ent);

		world.Update(pig::Timestep(0));

		auto viewAfter = pig::World::GetRegistryDirect().view<pig::EngineConfigSingletonComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded config has the three required UUID fields populated
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::LoadedConfigHasNonNullUUIDs")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ConfigLoaderSystem>());

		world.Update(pig::Timestep(0));

		auto view = pig::World::GetRegistryDirect().view<pig::EngineConfigSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pig::EngineConfigSingletonComponent& cfg =
			view.get<pig::EngineConfigSingletonComponent>(view.front());

		CHECK(!cfg.m_DefaultQuadShaderID.IsNull());
		CHECK(!cfg.m_DefaultTextShaderID.IsNull());
		CHECK(!cfg.m_DefaultFontID.IsNull());
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ConfigLoaderSystem::DeclareAccessIsCorrect")
	{
		pig::ConfigLoaderSystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pig::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pig::EngineConfigSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
