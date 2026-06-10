#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIEventSystem.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard condition: missing InputStateSingletonComponent -> no events emitted
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::DoesNothingWithoutInputState")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		// RendererConfig exists but no InputState.
		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 0.f,   0.f   };
		base.m_UUID    = pg::UUID::Generate();

		world.Update(pg::Timestep(0));

		auto viewHover = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
		CHECK(viewHover.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard condition: missing RendererConfigSingletonComponent -> no events
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::DoesNothingWithoutRendererConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);
		input.m_MousePos = { 10.f, 10.f };

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 0.f,   0.f   };
		base.m_UUID    = pg::UUID::Generate();

		world.Update(pg::Timestep(0));

		auto viewHover = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
		CHECK(viewHover.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: mouse inside bounds -> UIOnHoverOneFrameComponent emitted
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::HoverWhenMouseInsideBounds")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 10.f,  20.f  };
		base.m_UUID    = pg::UUID::Generate();
		base.m_Parent  = pg::ecs::null;

		// Mouse outside bounds — just outside top-left corner.
		input.m_MousePos = { 9.f, 19.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 0);
		}

		// Mouse inside bounds.
		input.m_MousePos = { 11.f, 21.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
			const pg::ui::UIOnHoverOneFrameComponent& hover =
				view.get<pg::ui::UIOnHoverOneFrameComponent>(view.front());
			CHECK(hover.m_ElementID == base.m_UUID);
		}
	}

	// ---------------------------------------------------------------------------
	// Edge case: mouse exactly at bottom-right corner (inclusive) triggers hover
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::HoverAtBoundsCorners")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		// Element at (10, 20) with size (200, 100) -> bounds x:[10,210], y:[20,120]
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 10.f,  20.f  };
		base.m_UUID    = pg::UUID::Generate();
		base.m_Parent  = pg::ecs::null;

		// Bottom-right corner is inclusive.
		input.m_MousePos = { 210.f, 120.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
		}

		// One pixel outside bottom-right — no hover.
		input.m_MousePos = { 211.f, 121.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Happy path: left button pressed while inside bounds -> UIOnClickOneFrameComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::ClickWhenMousePressedInsideBounds")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 10.f,  20.f  };
		base.m_UUID    = pg::UUID::Generate();
		base.m_Parent  = pg::ecs::null;

		// Mouse inside, button pressed.
		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysPressed[pg::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pg::Timestep(0));
		{
			auto viewClick = pg::World::GetRegistryDirect().view<pg::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(viewClick.size() == 1);
			REQUIRE(viewHover.size() == 1);
			const pg::ui::UIOnClickOneFrameComponent& click =
				viewClick.get<pg::ui::UIOnClickOneFrameComponent>(viewClick.front());
			CHECK(click.m_ElementID == base.m_UUID);
		}
	}

	// ---------------------------------------------------------------------------
	// Happy path: button pressed then released while inside -> UIOnReleaseOneFrameComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::ReleaseWhenMouseReleasedInsideBounds")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 10.f,  20.f  };
		base.m_UUID    = pg::UUID::Generate();
		base.m_Parent  = pg::ecs::null;

		// Simulate release: key in released map, not in pressed.
		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysReleased[pg::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pg::Timestep(0));
		{
			auto viewRelease = pg::World::GetRegistryDirect().view<pg::ui::UIOnReleaseOneFrameComponent>();
			auto viewClick   = pg::World::GetRegistryDirect().view<pg::ui::UIOnClickOneFrameComponent>();
			REQUIRE(viewRelease.size() == 1);
			CHECK(viewClick.size() == 0);
			const pg::ui::UIOnReleaseOneFrameComponent& rel =
				viewRelease.get<pg::ui::UIOnReleaseOneFrameComponent>(viewRelease.front());
			CHECK(rel.m_ElementID == base.m_UUID);
		}
	}

	// ---------------------------------------------------------------------------
	// Guard: click outside bounds -> no click event
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::NoClickWhenMouseOutsideBounds")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 10.f,  20.f  };
		base.m_UUID    = pg::UUID::Generate();
		base.m_Parent  = pg::ecs::null;

		// Mouse is outside the element bounds.
		input.m_MousePos = { 5.f, 5.f };
		input.m_KeysPressed[pg::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pg::Timestep(0));
		{
			auto viewClick = pg::World::GetRegistryDirect().view<pg::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(viewClick.size() == 0);
			CHECK(viewHover.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Guard: disabled element -> no events even when mouse is inside bounds
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::NoEventsForDisabledElement")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 10.f,  20.f  };
		base.m_UUID    = pg::UUID::Generate();
		base.m_Parent  = pg::ecs::null;
		base.m_Enabled = false;

		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysPressed[pg::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pg::Timestep(0));
		{
			auto viewClick = pg::World::GetRegistryDirect().view<pg::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(viewClick.size() == 0);
			CHECK(viewHover.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Guard: parent disabled -> child element produces no events
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::NoEventsWhenParentDisabled")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		pg::ecs::Entity parentEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& parentBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(parentEnt);
		parentBase.m_Enabled = false;
		parentBase.m_Size    = { 500.f, 500.f };
		parentBase.m_AnchoredPosition = { 0.f,   0.f   };

		pg::ecs::Entity childEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& childBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(childEnt);
		childBase.m_Enabled = true;
		childBase.m_Size    = { 200.f, 100.f };
		childBase.m_AnchoredPosition = { 10.f,  20.f  };
		childBase.m_Parent  = parentEnt;
		childBase.m_UUID    = pg::UUID::Generate();

		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysPressed[pg::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pg::Timestep(0));
		{
			auto viewClick = pg::World::GetRegistryDirect().view<pg::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(viewClick.size() == 0);
			CHECK(viewHover.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Resolution independence: the window-pixel mouse is converted into canvas units
	// (divided by the UI scale factor) before hit-testing, so a scaled window still
	// hits the element drawn in canvas space.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::HitTestTransformsMouseByScaleFactor")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		// Window is 2x the canvas: a scale factor of 2 means window pixels are halved to canvas units.
		cfg.m_Width       = 3840.f;
		cfg.m_Height      = 2160.f;
		cfg.m_ScaleFactor = 2.f;

		// Element at canvas (0,0) size (200,100) -> canvas bounds x:[0,200], y:[0,100].
		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size    = { 200.f, 100.f };
		base.m_AnchoredPosition = { 0.f,   0.f   };
		base.m_UUID    = pg::UUID::Generate();
		base.m_Parent  = pg::ecs::null;

		// Window pixel (300,150) -> canvas (150,75): inside the element.
		input.m_MousePos = { 300.f, 150.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
		}

		// Window pixel (500,150) -> canvas (250,75): x is outside the element's 200-wide canvas bounds.
		input.m_MousePos = { 500.f, 150.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Anchor-rect (stretch): an element anchored to fill its parent (anchorMin != anchorMax)
	// with uniform margins resolves to the inset rect; the margin band is outside it.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::StretchAnchorFillsParentWithMargins")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		// Fill the 1000x1000 canvas with a uniform 50px margin -> rect spans (50,50)-(950,950).
		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_AnchorMin        = { 0.f, 0.f };
		base.m_AnchorMax        = { 1.f, 1.f };
		base.m_Pivot            = { 0.f, 0.f };
		base.m_AnchoredPosition = { 50.f, 50.f };
		base.m_Size             = { -100.f, -100.f };
		base.m_UUID             = pg::UUID::Generate();

		input.m_MousePos = { 500.f, 500.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 1);
		}

		// Inside the left margin band (x = 25 < 50) -> outside the rect.
		input.m_MousePos = { 25.f, 500.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Anchor-rect (point + pivot): a centered element with a centered pivot resolves
	// to a rect centered on the anchor point.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::CenterPivotCentersRect")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		// Centered 200x100 box -> rect spans (400,450)-(600,550) around the canvas center.
		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_AnchorMin        = { 0.5f, 0.5f };
		base.m_AnchorMax        = { 0.5f, 0.5f };
		base.m_Pivot            = { 0.5f, 0.5f };
		base.m_AnchoredPosition = { 0.f, 0.f };
		base.m_Size             = { 200.f, 100.f };
		base.m_UUID             = pg::UUID::Generate();

		input.m_MousePos = { 500.f, 500.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 1);
		}

		// Just left of the rect's left edge (x = 399 < 400) -> outside.
		input.m_MousePos = { 399.f, 500.f };
		world.Update(pg::Timestep(0));
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			CHECK(view.size() == 0);
		}
	}

	// ---------------------------------------------------------------------------
	// Top-most input capture: with two overlapping enabled elements, only the
	// front-most (deeper-nested) one receives hover/click — events do not leak to
	// the element behind it (ADR 0005).
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::OnlyFrontMostElementReceivesEvents")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		// Parent covers (0,0)-(500,500); child (nested) covers (0,0)-(100,100). Both contain (50,50).
		const pg::UUID parentUUID = pg::UUID::Generate();
		const pg::UUID childUUID  = pg::UUID::Generate();

		pg::ecs::Entity parentEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& parentBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(parentEnt);
		parentBase.m_Size = { 500.f, 500.f };
		parentBase.m_UUID = parentUUID;

		pg::ecs::Entity childEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& childBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(childEnt);
		childBase.m_Size   = { 100.f, 100.f };
		childBase.m_Parent = parentEnt;
		childBase.m_UUID   = childUUID;

		input.m_MousePos = { 50.f, 50.f };
		input.m_KeysPressed[pg::PG_MOUSE_BUTTON_LEFT] = 1;

		world.Update(pg::Timestep(0));
		{
			auto viewHover = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			auto viewClick = pg::World::GetRegistryDirect().view<pg::ui::UIOnClickOneFrameComponent>();
			REQUIRE(viewHover.size() == 1);
			REQUIRE(viewClick.size() == 1);
			// Only the front-most (child) element is targeted; the parent behind it gets nothing.
			CHECK(viewHover.get<pg::ui::UIOnHoverOneFrameComponent>(viewHover.front()).m_ElementID == childUUID);
			CHECK(viewClick.get<pg::ui::UIOnClickOneFrameComponent>(viewClick.front()).m_ElementID == childUUID);
		}
	}

	// ---------------------------------------------------------------------------
	// Input capture: an interactive (image/text) element under the pointer raises a
	// UIInputCapturedInFrameEvent carrying its UUID, so gameplay can ignore the pointer.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::CaptureEventRaisedOverInteractiveElement")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		const pg::UUID elementUUID = pg::UUID::Generate();
		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size = { 200.f, 100.f };
		base.m_UUID = elementUUID;
		// ImageComponent is added by UIControlSystem in production; supplied here to mark the element interactive.
		pg::World::GetRegistryDirect().emplace<pg::ui::ImageComponent>(uiEnt).m_TextureHandle = pg::UUID::Generate();

		input.m_MousePos = { 50.f, 50.f };

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ui::UIInputCapturedInFrameEvent>();
		REQUIRE(view.size() == 1);
		CHECK(view.get<pg::ui::UIInputCapturedInFrameEvent>(view.front()).m_ElementID == elementUUID);
	}

	// ---------------------------------------------------------------------------
	// Input capture: a bare layout container (no image/text) does NOT raise capture,
	// so gameplay still works over empty UI areas — but it still hovers normally.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::NoCaptureOverBareContainer")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Size = { 200.f, 100.f };
		base.m_UUID = pg::UUID::Generate();

		input.m_MousePos = { 50.f, 50.f };

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto viewCapture = pg::World::GetRegistryDirect().view<pg::ui::UIInputCapturedInFrameEvent>();
		auto viewHover   = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
		CHECK(viewCapture.size() == 0);
		CHECK(viewHover.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Layout container (vertical stack): children are stacked top-to-bottom inside
	// the parent's padding with inter-item spacing, stretched to the content width;
	// probed via hit-testing of the resolved child slots.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::VerticalStackLaysOutChildren")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		// Parent rect (0,0,300,600); vertical stack, 10px padding all sides, 5px spacing.
		const pg::UUID parentUUID = pg::UUID::Generate();
		const pg::UUID child0UUID = pg::UUID::Generate();
		const pg::UUID child1UUID = pg::UUID::Generate();

		pg::ecs::Entity parentEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& parentBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(parentEnt);
		parentBase.m_Size = { 300.f, 600.f };
		parentBase.m_UUID = parentUUID;
		pg::ui::LayoutContainerComponent& container =
			pg::World::GetRegistryDirect().emplace<pg::ui::LayoutContainerComponent>(parentEnt);
		container.m_Type    = pg::ui::ELayoutType::eVertical;
		container.m_Padding = { 10.f, 10.f, 10.f, 10.f };
		container.m_Spacing = { 0.f, 5.f };

		pg::ecs::Entity child0 = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& c0 = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(child0);
		c0.m_Size = { 999.f, 100.f };  // width is overridden by cross-axis stretch
		c0.m_Parent = parentEnt;
		c0.m_SiblingIndex = 0;
		c0.m_UUID = child0UUID;

		pg::ecs::Entity child1 = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& c1 = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(child1);
		c1.m_Size = { 999.f, 80.f };
		c1.m_Parent = parentEnt;
		c1.m_SiblingIndex = 1;
		c1.m_UUID = child1UUID;

		// child0 slot: (10,10,280,100) -> y[10,110]; child1 slot: (10,115,280,80) -> y[115,195].
		auto hoveredUUID = [&]() -> pg::UUID
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
			return view.get<pg::ui::UIOnHoverOneFrameComponent>(view.front()).m_ElementID;
		};

		input.m_MousePos = { 150.f, 50.f };
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == child0UUID);

		input.m_MousePos = { 150.f, 150.f };
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == child1UUID);

		// In the 5px gap between the two child slots (y=112) -> only the parent container is hit.
		input.m_MousePos = { 150.f, 112.f };
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == parentUUID);
	}

	// ---------------------------------------------------------------------------
	// Layout container (horizontal stack): children are placed left-to-right inside
	// the padding with spacing, stretched to the content height.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::HorizontalStackLaysOutChildren")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		// Parent rect (0,0,600,300); horizontal stack, 10px padding, 5px spacing.
		const pg::UUID parentUUID = pg::UUID::Generate();
		const pg::UUID child0UUID = pg::UUID::Generate();
		const pg::UUID child1UUID = pg::UUID::Generate();

		pg::ecs::Entity parentEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& parentBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(parentEnt);
		parentBase.m_Size = { 600.f, 300.f };
		parentBase.m_UUID = parentUUID;
		pg::ui::LayoutContainerComponent& container =
			pg::World::GetRegistryDirect().emplace<pg::ui::LayoutContainerComponent>(parentEnt);
		container.m_Type    = pg::ui::ELayoutType::eHorizontal;
		container.m_Padding = { 10.f, 10.f, 10.f, 10.f };
		container.m_Spacing = { 5.f, 0.f };

		pg::ecs::Entity child0 = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& c0 = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(child0);
		c0.m_Size = { 100.f, 999.f };  // height is overridden by cross-axis stretch
		c0.m_Parent = parentEnt;
		c0.m_SiblingIndex = 0;
		c0.m_UUID = child0UUID;

		pg::ecs::Entity child1 = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& c1 = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(child1);
		c1.m_Size = { 80.f, 999.f };
		c1.m_Parent = parentEnt;
		c1.m_SiblingIndex = 1;
		c1.m_UUID = child1UUID;

		// child0 slot: (10,10,100,280) -> x[10,110]; child1 slot: (115,10,80,280) -> x[115,195].
		auto hoveredUUID = [&]() -> pg::UUID
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
			return view.get<pg::ui::UIOnHoverOneFrameComponent>(view.front()).m_ElementID;
		};

		input.m_MousePos = { 50.f, 150.f };
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == child0UUID);

		input.m_MousePos = { 150.f, 150.f };
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == child1UUID);

		// In the 5px gap between the two child slots (x=112) -> only the parent container is hit.
		input.m_MousePos = { 112.f, 150.f };
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == parentUUID);
	}

	// ---------------------------------------------------------------------------
	// Layout container (grid): children fill fixed cells, wrapping to the next row
	// after m_Columns columns, with inter-cell spacing.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::GridLaysOutChildrenInCells")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		pg::ecs::Entity parentEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& parentBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(parentEnt);
		parentBase.m_Size = { 1000.f, 1000.f };
		parentBase.m_UUID = pg::UUID::Generate();
		pg::ui::LayoutContainerComponent& container =
			pg::World::GetRegistryDirect().emplace<pg::ui::LayoutContainerComponent>(parentEnt);
		container.m_Type     = pg::ui::ELayoutType::eGrid;
		container.m_Columns  = 2;
		container.m_CellSize = { 100.f, 50.f };
		container.m_Spacing  = { 10.f, 10.f };

		// 4 cells: idx0=(0,0,100,50) idx1=(110,0,100,50) idx2=(0,60,100,50) idx3=(110,60,100,50).
		pg::UUID cellUUID[4];
		for (int i = 0; i < 4; ++i)
		{
			cellUUID[i] = pg::UUID::Generate();
			pg::ecs::Entity cellEnt = pg::World::GetRegistryDirect().create();
			pg::ui::BaseComponent& cell = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(cellEnt);
			cell.m_Parent = parentEnt;
			cell.m_SiblingIndex = i;
			cell.m_UUID = cellUUID[i];
		}

		auto hoveredUUID = [&]() -> pg::UUID
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
			return view.get<pg::ui::UIOnHoverOneFrameComponent>(view.front()).m_ElementID;
		};

		input.m_MousePos = { 50.f, 25.f };   // cell 0
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == cellUUID[0]);

		input.m_MousePos = { 160.f, 25.f };  // cell 1 (second column)
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == cellUUID[1]);

		input.m_MousePos = { 50.f, 85.f };   // cell 2 (second row)
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == cellUUID[2]);

		input.m_MousePos = { 160.f, 85.f };  // cell 3
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == cellUUID[3]);
	}

	// ---------------------------------------------------------------------------
	// Clip scroll offset: a UIClipComponent's scroll offset translates its children;
	// probed via hit-testing the shifted child slot.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::ClipScrollOffsetShiftsChildren")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIEventSystem>());

		pg::ecs::Entity inputEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& input =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(inputEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1000.f;
		cfg.m_Height = 1000.f;

		// Clip parent rect (0,0,300,300), scrolled up 50px.
		const pg::UUID parentUUID = pg::UUID::Generate();
		const pg::UUID childUUID  = pg::UUID::Generate();

		pg::ecs::Entity parentEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& parentBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(parentEnt);
		parentBase.m_Size = { 300.f, 300.f };
		parentBase.m_UUID = parentUUID;
		pg::World::GetRegistryDirect().emplace<pg::ui::UIClipComponent>(parentEnt).m_ScrollOffset = { 0.f, -50.f };

		// Child anchored at (50,50) size 100x100 -> unscrolled rect (50,50,100,100); scrolled to (50,0,100,100).
		pg::ecs::Entity childEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& childBase =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(childEnt);
		childBase.m_AnchoredPosition = { 50.f, 50.f };
		childBase.m_Size   = { 100.f, 100.f };
		childBase.m_Parent = parentEnt;
		childBase.m_UUID   = childUUID;

		auto hoveredUUID = [&]() -> pg::UUID
		{
			auto view = pg::World::GetRegistryDirect().view<pg::ui::UIOnHoverOneFrameComponent>();
			REQUIRE(view.size() == 1);
			return view.get<pg::ui::UIOnHoverOneFrameComponent>(view.front()).m_ElementID;
		};

		// (100,25): inside the scrolled child (y[0,100]); would be ABOVE the unscrolled child (y[50,150]).
		input.m_MousePos = { 100.f, 25.f };
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == childUUID);

		// (100,125): below the scrolled child -> only the parent is hit (the child moved up out of here).
		input.m_MousePos = { 100.f, 125.f };
		world.Update(pg::Timestep(0));
		CHECK(hoveredUUID() == parentUUID);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets contain the right component types
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIEventSystem::DeclareAccessIsCorrect")
	{
		pg::ui::UIEventSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::RendererConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIOnClickOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent))) > 0);

		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::ui::UIInputCapturedInFrameEvent))) > 0);
	}

} // namespace CatchTestsetFail
