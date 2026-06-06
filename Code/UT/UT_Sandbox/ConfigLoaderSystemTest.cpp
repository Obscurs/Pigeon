#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/ConfigLoaderSystem.h"
#include "Sandbox/SampleUIConfigSingletonComponent.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no SampleUIConfigSingletonComponent ->
	// system creates one via deferred add. Visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ConfigLoaderSystem::CreatesConfigOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ConfigLoaderSystem>());

		auto viewBefore = pg::World::GetRegistryDirect().view<sbx::SampleUIConfigSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<sbx::SampleUIConfigSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with SampleUIConfigSingletonComponent already present ->
	// system does nothing (no duplicate created)
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ConfigLoaderSystem::DoesNotDuplicateWhenConfigExists")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ConfigLoaderSystem>());

		// First frame: the system itself creates the SampleUIConfigSingletonComponent
		// (the test must not pre-create a component the system adds).
		world.Update(pg::Timestep(0));

		// Second frame: the config already exists, so the guard fires and no duplicate is added.
		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<sbx::SampleUIConfigSingletonComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded config has the four required UUID fields populated
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ConfigLoaderSystem::LoadedConfigHasNonNullUUIDs")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<sbx::SampleUIConfigSingletonComponent>();
		REQUIRE(view.size() == 1);

		const sbx::SampleUIConfigSingletonComponent& cfg =
			view.get<sbx::SampleUIConfigSingletonComponent>(view.front());

		CHECK(!cfg.m_UUIDUI1.IsNull());
		CHECK(!cfg.m_UUIDUI2.IsNull());
		CHECK(!cfg.m_DefaultFontID.IsNull());
		CHECK(!cfg.m_MainLayoutID.IsNull());
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ConfigLoaderSystem::DeclareAccessIsCorrect")
	{
		sbx::ConfigLoaderSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SampleUIConfigSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::SampleUIConfigSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
