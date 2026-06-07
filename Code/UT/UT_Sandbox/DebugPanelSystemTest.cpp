#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/DebugControlsSingletonComponent.h"
#include "Sandbox/DebugPanelSystem.h"
#include "Sandbox/SpawnerSingletonComponent.h"

#include <cmath>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first frame creates the controls singleton with its default speed.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DebugPanelSystem::CreatesControlsOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::DebugPanelSystem>());

		auto viewBefore = pg::World::GetRegistryDirect().view<sbx::DebugControlsSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<sbx::DebugControlsSingletonComponent>();
		REQUIRE(view.size() == 1);
		CHECK(std::fabs(view.get<sbx::DebugControlsSingletonComponent>(view.front()).m_AnimationSpeed - 1.f) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Guard: subsequent frames do not duplicate the controls singleton (and the ImGui
	// calls are skipped because the test build has no ImGui context).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DebugPanelSystem::DoesNotDuplicateControls")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::DebugPanelSystem>());

		world.Update(pg::Timestep(0));
		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::DebugControlsSingletonComponent>().size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DebugPanelSystem::DeclareAccessIsCorrect")
	{
		sbx::DebugPanelSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SpawnerSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::DebugControlsSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::DebugControlsSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
