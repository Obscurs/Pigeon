#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/SaveDataSingletonComponent.h"
#include "Pigeon/Core/SetSaveDataRequestOneFrameComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/SaveDataDebugPanelSystem.h"
#include "Sandbox/SaveDataPanelStateSingletonComponent.h"

namespace CatchTestsetFail
{
	// Guard: with no savedata singleton present the system does nothing and emits no request.
	TEST_CASE("Sandbox.SaveDataDebugPanelSystem::NoOpWithoutSaveData")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SaveDataDebugPanelSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::SaveDataPanelStateSingletonComponent>().size() == 0);
		CHECK(pg::World::GetRegistryDirect().view<pg::SetSaveDataRequestOneFrameComponent>().size() == 0);
	}

	// Seeding: with the savedata singleton present the system creates its panel-state singleton once.
	TEST_CASE("Sandbox.SaveDataDebugPanelSystem::SeedsPanelState")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SaveDataDebugPanelSystem>());

		pg::ecs::Entity sdEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SaveDataSingletonComponent>(sdEnt);

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::SaveDataPanelStateSingletonComponent>().size() == 1);
	}

	// Guard: the test build has no ImGui context, so even with the singleton present no request is emitted.
	TEST_CASE("Sandbox.SaveDataDebugPanelSystem::NoRequestWithoutImGuiContext")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SaveDataDebugPanelSystem>());

		pg::ecs::Entity sdEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SaveDataSingletonComponent>(sdEnt);

		world.Update(pg::Timestep(0)); // seeds the panel state
		world.Update(pg::Timestep(0)); // would draw, but no ImGui context

		CHECK(pg::World::GetRegistryDirect().view<pg::SetSaveDataRequestOneFrameComponent>().size() == 0);
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Sandbox.SaveDataDebugPanelSystem::DeclareAccessIsCorrect")
	{
		sbx::SaveDataDebugPanelSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::SaveDataSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::SaveDataPanelStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::SaveDataPanelStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::SetSaveDataRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
