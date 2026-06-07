#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UIButtonVisualStateSingletonComponent.h"
#include "Sandbox/UIButtonVisualSystem.h"

namespace
{
	sbx::SandboxConfigSingletonComponent& SeedButtonConfig(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity cfgEnt = registry.create();
		sbx::SandboxConfigSingletonComponent& cfg = registry.emplace<sbx::SandboxConfigSingletonComponent>(cfgEnt);
		cfg.m_ToggleButtonID = pg::UUID::Generate();
		cfg.m_ButtonImageID = pg::UUID::Generate();
		cfg.m_ButtonHoverImageID = pg::UUID::Generate();
		cfg.m_ButtonPressedImageID = pg::UUID::Generate();
		return cfg;
	}

	// BaseComponent is added in production by UIControlSystem (a different system).
	pg::ecs::Entity MakeButton(pg::ecs::Registry& registry, const pg::UUID& uuid)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<pg::ui::BaseComponent>(ent).m_UUID = uuid;
		return ent;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: without config the system creates nothing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIButtonVisualSystem::NoOpWithoutConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIButtonVisualSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::UIButtonVisualStateSingletonComponent>().size() == 0);
		CHECK(pg::World::GetRegistryDirect().view<pg::ui::UIUpdateImageUUIDOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: an idle button is given its default image (and the state singleton appears).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIButtonVisualSystem::EmitsDefaultImageWhenIdle")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIButtonVisualSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedButtonConfig(pg::World::GetRegistryDirect());
		const pg::UUID defaultImage = cfg.m_ButtonImageID;
		const pg::UUID buttonID = cfg.m_ToggleButtonID;

		world.Update(pg::Timestep(0)); // lazily creates the visual-state singleton
		REQUIRE(pg::World::GetRegistryDirect().view<sbx::UIButtonVisualStateSingletonComponent>().size() == 1);

		pg::ecs::Entity button = MakeButton(pg::World::GetRegistryDirect(), buttonID);

		world.Update(pg::Timestep(0));

		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateImageUUIDOneFrameComponent>(button));
		CHECK(pg::World::GetRegistryDirect().get<pg::ui::UIUpdateImageUUIDOneFrameComponent>(button).m_UUID == defaultImage);
	}

	// ---------------------------------------------------------------------------
	// Happy path: a hovered button is given its hover image.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIButtonVisualSystem::EmitsHoverImageWhenHovered")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIButtonVisualSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedButtonConfig(pg::World::GetRegistryDirect());
		const pg::UUID hoverImage = cfg.m_ButtonHoverImageID;
		const pg::UUID buttonID = cfg.m_ToggleButtonID;

		world.Update(pg::Timestep(0)); // create state singleton

		pg::ecs::Entity button = MakeButton(pg::World::GetRegistryDirect(), buttonID);
		// UIOnHover is added in production by UIEventSystem (a different system).
		pg::World::GetRegistryDirect().emplace<pg::ui::UIOnHoverOneFrameComponent>(button).m_ElementID = buttonID;

		world.Update(pg::Timestep(0));

		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateImageUUIDOneFrameComponent>(button));
		CHECK(pg::World::GetRegistryDirect().get<pg::ui::UIUpdateImageUUIDOneFrameComponent>(button).m_UUID == hoverImage);
	}

	// ---------------------------------------------------------------------------
	// Happy path: a pressed button takes the pressed image (press beats hover).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIButtonVisualSystem::EmitsPressedImageWhenClicked")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIButtonVisualSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedButtonConfig(pg::World::GetRegistryDirect());
		const pg::UUID pressedImage = cfg.m_ButtonPressedImageID;
		const pg::UUID buttonID = cfg.m_ToggleButtonID;

		world.Update(pg::Timestep(0)); // create state singleton

		pg::ecs::Entity button = MakeButton(pg::World::GetRegistryDirect(), buttonID);
		pg::World::GetRegistryDirect().emplace<pg::ui::UIOnHoverOneFrameComponent>(button).m_ElementID = buttonID;
		pg::World::GetRegistryDirect().emplace<pg::ui::UIOnClickOneFrameComponent>(button).m_ElementID = buttonID;

		world.Update(pg::Timestep(0));

		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateImageUUIDOneFrameComponent>(button));
		CHECK(pg::World::GetRegistryDirect().get<pg::ui::UIUpdateImageUUIDOneFrameComponent>(button).m_UUID == pressedImage);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIButtonVisualSystem::DeclareAccessIsCorrect")
	{
		sbx::UIButtonVisualSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIOnClickOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::UIButtonVisualStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIUpdateImageUUIDOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::UIButtonVisualStateSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
