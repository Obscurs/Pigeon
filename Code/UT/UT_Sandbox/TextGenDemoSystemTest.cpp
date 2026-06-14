#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/TextGen/GenerateTextRequestOneFrameComponent.h"
#include "Pigeon/TextGen/TextGenJobSingletonComponent.h"
#include "Pigeon/TextGen/TextGenResultSingletonComponent.h"
#include "Sandbox/TextGenDemoStateSingletonComponent.h"
#include "Sandbox/TextGenDemoSystem.h"

namespace CatchTestsetFail
{
	// Seeds the engine TextGen singletons the demo reads (added by TextGenSystem in production).
	void SeedTextGenSingletons()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		registry.emplace<pg::TextGenJobSingletonComponent>(registry.create());
		registry.emplace<pg::TextGenResultSingletonComponent>(registry.create());
	}

	// Guard: with no TextGen singletons present the system does nothing and emits no request.
	TEST_CASE("Sandbox.TextGenDemoSystem::NoOpWithoutTextGenSingletons")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TextGenDemoSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::GenerateTextRequestOneFrameComponent>().size() == 0);
	}

	// Guard: the test build has no ImGui context, so even with the singletons present no request is emitted.
	TEST_CASE("Sandbox.TextGenDemoSystem::NoRequestWithoutImGuiContext")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TextGenDemoSystem>());
		SeedTextGenSingletons();

		world.Update(pg::Timestep(0)); // seeds demo state, returns
		world.Update(pg::Timestep(0)); // ImGui guard -> no request

		CHECK(pg::World::GetRegistryDirect().view<pg::GenerateTextRequestOneFrameComponent>().size() == 0);
	}

	// The demo seeds its editable state singleton once from the engine defaults.
	TEST_CASE("Sandbox.TextGenDemoSystem::SeedsDemoStateSingleton")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TextGenDemoSystem>());
		SeedTextGenSingletons();

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::TextGenDemoStateSingletonComponent>().size() == 1);
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Sandbox.TextGenDemoSystem::DeclareAccessIsCorrect")
	{
		sbx::TextGenDemoSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::TextGenResultSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::TextGenJobSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::TextGenDemoStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::TextGenDemoStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::GenerateTextRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
