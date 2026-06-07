#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/LifetimeComponent.h"
#include "Sandbox/LifetimeSystem.h"

#include <cmath>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: remaining time is reduced by the frame delta; entity survives.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.LifetimeSystem::DecrementsRemaining")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::LifetimeSystem>());

		// LifetimeComponent is added in production by QuadSpawnSystem (a different system).
		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::LifetimeComponent>(ent).m_Remaining = 5.f;

		world.Update(pg::Timestep(1000)); // 1 second

		REQUIRE(pg::World::GetRegistryDirect().valid(ent));
		const sbx::LifetimeComponent& result = pg::World::GetRegistryDirect().get<sbx::LifetimeComponent>(ent);
		CHECK(std::fabs(result.m_Remaining - 4.f) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: an entity whose remaining time hits zero is destroyed.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.LifetimeSystem::DestroysWhenExpired")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::LifetimeSystem>());

		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::LifetimeComponent>(ent).m_Remaining = 0.5f;

		world.Update(pg::Timestep(1000)); // exceeds remaining

		CHECK(!pg::World::GetRegistryDirect().valid(ent));
	}

	// ---------------------------------------------------------------------------
	// Edge case: remaining reaching exactly zero destroys the entity.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.LifetimeSystem::DestroysAtExactlyZero")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::LifetimeSystem>());

		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::LifetimeComponent>(ent).m_Remaining = 1.f;

		world.Update(pg::Timestep(1000)); // remaining -> 0

		CHECK(!pg::World::GetRegistryDirect().valid(ent));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.LifetimeSystem::DeclareAccessIsCorrect")
	{
		sbx::LifetimeSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::LifetimeComponent))) > 0);
	}

} // namespace CatchTestsetFail
