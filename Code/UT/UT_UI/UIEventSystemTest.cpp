#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/InputStateSingletonComponent.h>
#include <Pigeon/Core/KeyCodes.h>
#include <Pigeon/UI/UIComponents.h>
#include <Pigeon/UI/UIEventSystem.h>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard condition: missing InputStateSingletonComponent -> no events emitted
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::DoesNothingWithoutInputState")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		// RendererConfig exists but no InputState.
		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_Spacing = { 0.f,   0.f   };
		base.m_UUID    = pig::UUID::Generate();

		world.Update(pig::Timestep(0));

		auto viewHover = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
		CHECK(viewHover.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard condition: missing RendererConfigSingletonComponent -> no events
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::DoesNothingWithoutRendererConfig")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		entt::entity inputEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& input =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(inputEnt);
		input.m_MousePos = { 10.f, 10.f };

		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_Spacing = { 0.f,   0.f   };
		base.m_UUID    = pig::UUID::Generate();

		world.Update(pig::Timestep(0));

		auto viewHover = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
		CHECK(viewHover.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: mouse inside bounds -> UIOnHoverOneFrameComponent emitted
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::HoverWhenMouseInsideBounds")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		entt::entity inputEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& input =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(inputEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_Spacing = { 10.f,  20.f  };
		base.m_UUID    = pig::UUID::Generate();
		base.m_Parent  = entt::null;

		// Mouse outside bounds — just outside top-left corner.
		input.m_MousePos = { 9.f, 19.f };
		world.Update(pig::Timestep(0));
		{
			auto view = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 0);
		}

		// Mouse inside bounds.
		input.m_MousePos = { 11.f, 21.f };
		world.Update(pig::Timestep(0));
		{
			auto view = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
			const pig::ui::UIOnHoverOneFrameComponent& hover =
				view.get<pig::ui::UIOnHoverOneFrameComponent>(view.front());
			CHECK(hover.m_ElementID == base.m_UUID);
		}
	}

	// ---------------------------------------------------------------------------
	// Edge case: mouse exactly at bottom-right corner (inclusive) triggers hover
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::HoverAtBoundsCorners")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		entt::entity inputEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& input =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(inputEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		// Element at (10, 20) with size (200, 100) -> bounds x:[10,210], y:[20,120]
		base.m_Size    = { 200.f, 100.f };
		base.m_Spacing = { 10.f,  20.f  };
		base.m_UUID    = pig::UUID::Generate();
		base.m_Parent  = entt::null;

		// Bottom-right corner is inclusive.
		input.m_MousePos = { 210.f, 120.f };
		world.Update(pig::Timestep(0));
		{
			auto view = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
		}

		// One pixel outside bottom-right — no hover.
		input.m_MousePos = { 211.f, 121.f };
		world.Update(pig::Timestep(0));
		{
			auto view = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Happy path: left button pressed while inside bounds -> UIOnClickOneFrameComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::ClickWhenMousePressedInsideBounds")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		entt::entity inputEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& input =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(inputEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_Spacing = { 10.f,  20.f  };
		base.m_UUID    = pig::UUID::Generate();
		base.m_Parent  = entt::null;

		// Mouse inside, button pressed.
		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysPressed[pig::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pig::Timestep(0));
		{
			auto viewClick = pig::World::GetRegistryDirect().view<pig::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(viewClick.size() == 1);
			REQUIRE(viewHover.size() == 1);
			const pig::ui::UIOnClickOneFrameComponent& click =
				viewClick.get<pig::ui::UIOnClickOneFrameComponent>(viewClick.front());
			CHECK(click.m_ElementID == base.m_UUID);
		}
	}

	// ---------------------------------------------------------------------------
	// Happy path: button pressed then released while inside -> UIOnReleaseOneFrameComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::ReleaseWhenMouseReleasedInsideBounds")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		entt::entity inputEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& input =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(inputEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_Spacing = { 10.f,  20.f  };
		base.m_UUID    = pig::UUID::Generate();
		base.m_Parent  = entt::null;

		// Simulate release: key in released map, not in pressed.
		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysReleased[pig::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pig::Timestep(0));
		{
			auto viewRelease = pig::World::GetRegistryDirect().view<pig::ui::UIOnReleaseOneFrameComponent>();
			auto viewClick   = pig::World::GetRegistryDirect().view<pig::ui::UIOnClickOneFrameComponent>();
			REQUIRE(viewRelease.size() == 1);
			CHECK(viewClick.size() == 0);
			const pig::ui::UIOnReleaseOneFrameComponent& rel =
				viewRelease.get<pig::ui::UIOnReleaseOneFrameComponent>(viewRelease.front());
			CHECK(rel.m_ElementID == base.m_UUID);
		}
	}

	// ---------------------------------------------------------------------------
	// Guard: click outside bounds -> no click event
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::NoClickWhenMouseOutsideBounds")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		entt::entity inputEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& input =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(inputEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_Spacing = { 10.f,  20.f  };
		base.m_UUID    = pig::UUID::Generate();
		base.m_Parent  = entt::null;

		// Mouse is outside the element bounds.
		input.m_MousePos = { 5.f, 5.f };
		input.m_KeysPressed[pig::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pig::Timestep(0));
		{
			auto viewClick = pig::World::GetRegistryDirect().view<pig::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
			CHECK(viewClick.size() == 0);
			CHECK(viewHover.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Guard: disabled element -> no events even when mouse is inside bounds
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::NoEventsForDisabledElement")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		entt::entity inputEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& input =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(inputEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_Spacing = { 10.f,  20.f  };
		base.m_UUID    = pig::UUID::Generate();
		base.m_Parent  = entt::null;
		base.m_Enabled = false;

		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysPressed[pig::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pig::Timestep(0));
		{
			auto viewClick = pig::World::GetRegistryDirect().view<pig::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
			CHECK(viewClick.size() == 0);
			CHECK(viewHover.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Guard: parent disabled -> child element produces no events
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::NoEventsWhenParentDisabled")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIEventSystem>());

		entt::entity inputEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& input =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(inputEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		entt::entity parentEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& parentBase =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(parentEnt);
		parentBase.m_Enabled = false;
		parentBase.m_Size    = { 500.f, 500.f };
		parentBase.m_Spacing = { 0.f,   0.f   };

		entt::entity childEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& childBase =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(childEnt);
		childBase.m_Enabled = true;
		childBase.m_Size    = { 200.f, 100.f };
		childBase.m_Spacing = { 10.f,  20.f  };
		childBase.m_Parent  = parentEnt;
		childBase.m_UUID    = pig::UUID::Generate();

		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysPressed[pig::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pig::Timestep(0));
		{
			auto viewClick = pig::World::GetRegistryDirect().view<pig::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pig::World::GetRegistryDirect().view<pig::ui::UIOnHoverOneFrameComponent>();
			CHECK(viewClick.size() == 0);
			CHECK(viewHover.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets contain the right component types
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::DeclareAccessIsCorrect")
	{
		pig::ui::UIEventSystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pig::InputStateSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::RendererConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::BaseComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pig::ui::UIOnClickOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pig::ui::UIOnHoverOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pig::ui::UIOnReleaseOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
