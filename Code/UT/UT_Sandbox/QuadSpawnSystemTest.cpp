#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/LifetimeComponent.h"
#include "Sandbox/QuadComponent.h"
#include "Sandbox/QuadSpawnSystem.h"
#include "Sandbox/QuadSpawnTransformRequestOneFrameComponent.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/SpawnerSingletonComponent.h"
#include "Sandbox/SpinComponent.h"

namespace
{
	sbx::SandboxConfigSingletonComponent& SeedConfig(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity cfgEnt = registry.create();
		sbx::SandboxConfigSingletonComponent& cfg = registry.emplace<sbx::SandboxConfigSingletonComponent>(cfgEnt);
		cfg.m_TexturedQuadTextureID = pg::UUID::Generate();
		return cfg;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: without SandboxConfigSingletonComponent the system does nothing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadSpawnSystem::NoOpWithoutConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadSpawnSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::SpawnerSingletonComponent>().size() == 0);
		CHECK(pg::World::GetRegistryDirect().view<sbx::QuadComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: first ready frame seeds persistent quads and creates the spawner.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadSpawnSystem::SeedsSceneOnFirstReadyFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadSpawnSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedConfig(pg::World::GetRegistryDirect());
		const pg::UUID texturedID = cfg.m_TexturedQuadTextureID;

		world.Update(pg::Timestep(0));

		auto spawnerView = pg::World::GetRegistryDirect().view<sbx::SpawnerSingletonComponent>();
		REQUIRE(spawnerView.size() == 1);
		CHECK(spawnerView.get<sbx::SpawnerSingletonComponent>(spawnerView.front()).m_SpawnCount == 0);

		auto quadView = pg::World::GetRegistryDirect().view<sbx::QuadComponent>();
		auto spinView = pg::World::GetRegistryDirect().view<sbx::SpinComponent>();
		CHECK(quadView.size() > 0);
		CHECK(spinView.size() == quadView.size());
		// Each seeded quad requests its initial transform.
		CHECK(pg::World::GetRegistryDirect().view<sbx::QuadSpawnTransformRequestOneFrameComponent>().size() == quadView.size());
		// Seeded quads are persistent: no LifetimeComponent yet.
		CHECK(pg::World::GetRegistryDirect().view<sbx::LifetimeComponent>().size() == 0);

		bool foundTextured = false;
		bool foundFlat = false;
		for (pg::ecs::Entity ent : quadView)
		{
			const sbx::QuadComponent& quad = quadView.get<sbx::QuadComponent>(ent);
			if (quad.m_TextureID == texturedID)
			{
				foundTextured = true;
			}
			if (quad.m_TextureID.IsNull())
			{
				foundFlat = true;
			}
		}
		CHECK(foundTextured);
		CHECK(foundFlat);
	}

	// ---------------------------------------------------------------------------
	// Happy path: a fresh spacebar press spawns one temporary quad and bumps the count.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadSpawnSystem::SpawnsTemporaryQuadOnSpacePress")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadSpawnSystem>());

		SeedConfig(pg::World::GetRegistryDirect());
		world.Update(pg::Timestep(0)); // seed + create spawner

		const size_t seedCount = pg::World::GetRegistryDirect().view<sbx::QuadComponent>().size();

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);
		input.m_KeysPressed[pg::PG_KEY_SPACE] = 1; // pressed this frame

		world.Update(pg::Timestep(16));

		auto spawnerView = pg::World::GetRegistryDirect().view<sbx::SpawnerSingletonComponent>();
		REQUIRE(spawnerView.size() == 1);
		CHECK(spawnerView.get<sbx::SpawnerSingletonComponent>(spawnerView.front()).m_SpawnCount == 1);
		CHECK(pg::World::GetRegistryDirect().view<sbx::QuadComponent>().size() == seedCount + 1);
		CHECK(pg::World::GetRegistryDirect().view<sbx::LifetimeComponent>().size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Edge case: a held spacebar (already pressed in an earlier frame) does not spawn.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadSpawnSystem::DoesNotSpawnWhenSpaceHeld")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadSpawnSystem>());

		SeedConfig(pg::World::GetRegistryDirect());
		world.Update(pg::Timestep(0)); // seed + create spawner

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);
		input.m_KeysPressed[pg::PG_KEY_SPACE] = 2; // held since a previous frame

		world.Update(pg::Timestep(16));

		auto spawnerView = pg::World::GetRegistryDirect().view<sbx::SpawnerSingletonComponent>();
		REQUIRE(spawnerView.size() == 1);
		CHECK(spawnerView.get<sbx::SpawnerSingletonComponent>(spawnerView.front()).m_SpawnCount == 0);
		CHECK(pg::World::GetRegistryDirect().view<sbx::LifetimeComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadSpawnSystem::DeclareAccessIsCorrect")
	{
		sbx::QuadSpawnSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::SpawnerSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::QuadComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::QuadSpawnTransformRequestOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::SpinComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::LifetimeComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::SpawnerSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
