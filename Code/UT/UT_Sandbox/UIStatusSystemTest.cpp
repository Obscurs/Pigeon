#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/SpawnerSingletonComponent.h"
#include "Sandbox/StatusDisplaySingletonComponent.h"
#include "Sandbox/UIStatusSystem.h"

namespace
{
	sbx::SandboxConfigSingletonComponent& SeedStatusConfig(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity cfgEnt = registry.create();
		sbx::SandboxConfigSingletonComponent& cfg = registry.emplace<sbx::SandboxConfigSingletonComponent>(cfgEnt);
		cfg.m_StatusTextID = pg::UUID::Generate();
		cfg.m_DefaultFontID = pg::UUID::Generate();
		return cfg;
	}

	// SpawnerSingletonComponent is added in production by QuadSpawnSystem (a different system).
	pg::ecs::Entity SeedSpawner(pg::ecs::Registry& registry, int count)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<sbx::SpawnerSingletonComponent>(ent).m_SpawnCount = count;
		return ent;
	}

	pg::ecs::Entity MakeStatusText(pg::ecs::Registry& registry, const pg::UUID& uuid)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<pg::ui::BaseComponent>(ent).m_UUID = uuid;
		return ent;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: without a spawner the system does nothing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIStatusSystem::NoOpWithoutSpawner")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIStatusSystem>());

		SeedStatusConfig(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::StatusDisplaySingletonComponent>().size() == 0);
		CHECK(pg::World::GetRegistryDirect().view<pg::ui::UIUpdateTextOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the status text is updated with the current spawn count.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIStatusSystem::UpdatesStatusTextWithSpawnCount")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIStatusSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedStatusConfig(pg::World::GetRegistryDirect());
		const pg::UUID fontID = cfg.m_DefaultFontID;
		SeedSpawner(pg::World::GetRegistryDirect(), 3);

		world.Update(pg::Timestep(0)); // lazily creates the status-display singleton
		REQUIRE(pg::World::GetRegistryDirect().view<sbx::StatusDisplaySingletonComponent>().size() == 1);

		pg::ecs::Entity statusText = MakeStatusText(pg::World::GetRegistryDirect(), cfg.m_StatusTextID);

		world.Update(pg::Timestep(0));

		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateTextOneFrameComponent>(statusText));
		const pg::ui::UIUpdateTextOneFrameComponent& update =
			pg::World::GetRegistryDirect().get<pg::ui::UIUpdateTextOneFrameComponent>(statusText);
		CHECK(update.m_Text == "Spawned: 3");
		CHECK(update.m_FontID == fontID);
		CHECK(update.m_Color == glm::vec4(1.f, 1.f, 1.f, 1.f));
		CHECK(std::fabs(update.m_Kerning - 0.1f) < 1e-4f);
		CHECK(std::fabs(update.m_Spacing - 0.1f) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: a changed spawn count produces a new status update.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIStatusSystem::UpdatesAgainWhenCountChanges")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIStatusSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedStatusConfig(pg::World::GetRegistryDirect());
		pg::ecs::Entity spawnerEnt = SeedSpawner(pg::World::GetRegistryDirect(), 0);

		world.Update(pg::Timestep(0)); // create display singleton
		pg::ecs::Entity statusText = MakeStatusText(pg::World::GetRegistryDirect(), cfg.m_StatusTextID);
		world.Update(pg::Timestep(0)); // emits "Spawned: 0"

		pg::World::GetRegistryDirect().get<sbx::SpawnerSingletonComponent>(spawnerEnt).m_SpawnCount = 5;

		world.Update(pg::Timestep(0));

		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateTextOneFrameComponent>(statusText));
		CHECK(pg::World::GetRegistryDirect().get<pg::ui::UIUpdateTextOneFrameComponent>(statusText).m_Text == "Spawned: 5");
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIStatusSystem::DeclareAccessIsCorrect")
	{
		sbx::UIStatusSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SpawnerSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::StatusDisplaySingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIUpdateTextOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::StatusDisplaySingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
