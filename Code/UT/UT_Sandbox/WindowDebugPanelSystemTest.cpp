#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/SetWindowResolutionRequestOneFrameComponent.h"
#include "Pigeon/Core/WindowConfigSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/WindowDebugPanelSystem.h"
#include "Sandbox/WindowPanelSelectionSingletonComponent.h"

namespace CatchTestsetFail
{
	// Guard: with no window-config singleton present the system does nothing and emits no request.
	TEST_CASE("Sandbox.WindowDebugPanelSystem::NoOpWithoutWindowConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::WindowDebugPanelSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::SetWindowResolutionRequestOneFrameComponent>().size() == 0);
	}

	// Guard: the test build has no ImGui context, so even with the singleton present no request is emitted.
	TEST_CASE("Sandbox.WindowDebugPanelSystem::NoRequestWithoutImGuiContext")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::WindowDebugPanelSystem>());

		pg::ecs::Entity wcEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::WindowConfigSingletonComponent>(wcEnt);

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::SetWindowResolutionRequestOneFrameComponent>().size() == 0);
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Sandbox.WindowDebugPanelSystem::DeclareAccessIsCorrect")
	{
		sbx::WindowDebugPanelSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::WindowConfigSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::WindowPanelSelectionSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::WindowPanelSelectionSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::SetWindowResolutionRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
