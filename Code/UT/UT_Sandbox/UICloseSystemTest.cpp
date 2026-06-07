#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UICloseSystem.h"

namespace
{
	sbx::SandboxConfigSingletonComponent& SeedCloseConfig(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity cfgEnt = registry.create();
		sbx::SandboxConfigSingletonComponent& cfg = registry.emplace<sbx::SandboxConfigSingletonComponent>(cfgEnt);
		cfg.m_CloseButtonID = pg::UUID::Generate();
		cfg.m_CloseTargetID = pg::UUID::Generate();
		return cfg;
	}

	pg::ecs::Entity MakeElement(pg::ecs::Registry& registry, const pg::UUID& uuid)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<pg::ui::BaseComponent>(ent).m_UUID = uuid;
		return ent;
	}

	void EmitRelease(pg::ecs::Registry& registry, const pg::UUID& elementID)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<pg::ui::UIOnReleaseOneFrameComponent>(ent).m_ElementID = elementID;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: without config the system does nothing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UICloseSystem::NoOpWithoutConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UICloseSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::ui::UIDestroyOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: releasing the close button marks the close-target for destruction.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UICloseSystem::DestroysTargetOnRelease")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UICloseSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedCloseConfig(pg::World::GetRegistryDirect());
		pg::ecs::Entity target = MakeElement(pg::World::GetRegistryDirect(), cfg.m_CloseTargetID);
		EmitRelease(pg::World::GetRegistryDirect(), cfg.m_CloseButtonID);

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().any_of<pg::ui::UIDestroyOneFrameComponent>(target));
	}

	// ---------------------------------------------------------------------------
	// Guard: no release event -> nothing is marked for destruction.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UICloseSystem::NoDestroyWithoutRelease")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UICloseSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedCloseConfig(pg::World::GetRegistryDirect());
		pg::ecs::Entity target = MakeElement(pg::World::GetRegistryDirect(), cfg.m_CloseTargetID);

		world.Update(pg::Timestep(0));

		CHECK(!pg::World::GetRegistryDirect().any_of<pg::ui::UIDestroyOneFrameComponent>(target));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UICloseSystem::DeclareAccessIsCorrect")
	{
		sbx::UICloseSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIDestroyOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
