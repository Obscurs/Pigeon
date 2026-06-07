#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UIButtonSystem.h"

namespace
{
	sbx::SandboxConfigSingletonComponent& SeedToggleConfig(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity cfgEnt = registry.create();
		sbx::SandboxConfigSingletonComponent& cfg = registry.emplace<sbx::SandboxConfigSingletonComponent>(cfgEnt);
		cfg.m_ToggleButtonID = pg::UUID::Generate();
		cfg.m_TogglePanelID = pg::UUID::Generate();
		return cfg;
	}

	pg::ecs::Entity MakePanel(pg::ecs::Registry& registry, const pg::UUID& uuid, bool enabled)
	{
		pg::ecs::Entity ent = registry.create();
		pg::ui::BaseComponent& base = registry.emplace<pg::ui::BaseComponent>(ent);
		base.m_UUID = uuid;
		base.m_Enabled = enabled;
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
	TEST_CASE("Sandbox.UIButtonSystem::NoOpWithoutConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIButtonSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::ui::UIUpdateEnableOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: releasing the toggle button flips the panel's enabled flag.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIButtonSystem::TogglesPanelOnRelease")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIButtonSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedToggleConfig(pg::World::GetRegistryDirect());
		pg::ecs::Entity panel = MakePanel(pg::World::GetRegistryDirect(), cfg.m_TogglePanelID, true);
		EmitRelease(pg::World::GetRegistryDirect(), cfg.m_ToggleButtonID);

		world.Update(pg::Timestep(0));

		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateEnableOneFrameComponent>(panel));
		CHECK(pg::World::GetRegistryDirect().get<pg::ui::UIUpdateEnableOneFrameComponent>(panel).m_Enabled == false);
	}

	// ---------------------------------------------------------------------------
	// Guard: no release event -> the panel is left untouched.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIButtonSystem::NoToggleWithoutRelease")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIButtonSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedToggleConfig(pg::World::GetRegistryDirect());
		pg::ecs::Entity panel = MakePanel(pg::World::GetRegistryDirect(), cfg.m_TogglePanelID, true);

		world.Update(pg::Timestep(0));

		CHECK(!pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateEnableOneFrameComponent>(panel));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIButtonSystem::DeclareAccessIsCorrect")
	{
		sbx::UIButtonSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIUpdateEnableOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
