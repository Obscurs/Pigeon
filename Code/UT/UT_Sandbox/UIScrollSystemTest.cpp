#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UIScrollSystem.h"

namespace CatchTestsetFail
{
	namespace
	{
		// Builds the common state the system reads (config + a clip panel with a current scroll offset),
		// returning the panel entity. The panel's BaseComponent UUID matches the config's scroll panel id.
		pg::ecs::Entity SeedPanel(const pg::UUID& panelID, float currentOffsetY)
		{
			pg::ecs::Registry& reg = pg::World::GetRegistryDirect();

			pg::ecs::Entity cfgEnt = reg.create();
			sbx::SandboxConfigSingletonComponent& cfg = reg.emplace<sbx::SandboxConfigSingletonComponent>(cfgEnt);
			cfg.m_ScrollPanelID = panelID;

			pg::ecs::Entity panelEnt = reg.create();
			pg::ui::BaseComponent& base = reg.emplace<pg::ui::BaseComponent>(panelEnt);
			base.m_UUID = panelID;
			pg::ui::UIClipComponent& clip = reg.emplace<pg::ui::UIClipComponent>(panelEnt);
			clip.m_ScrollOffset = { 0.f, currentOffsetY };

			return panelEnt;
		}

		void SeedScroll(float yOffset)
		{
			pg::ecs::Registry& reg = pg::World::GetRegistryDirect();
			pg::ecs::Entity ent = reg.create();
			reg.emplace<pg::MouseScrolledEventComponent>(ent).m_YOffset = yOffset;
		}

		void SeedHover(const pg::UUID& elementID)
		{
			pg::ecs::Registry& reg = pg::World::GetRegistryDirect();
			pg::ecs::Entity ent = reg.create();
			reg.emplace<pg::ui::UIOnHoverOneFrameComponent>(ent).m_ElementID = elementID;
		}
	}

	// ---------------------------------------------------------------------------
	// Happy path: wheel while hovering the panel accumulates the delta into the
	// panel's scroll offset and emits a clip-offset update on the panel.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIScrollSystem::ScrollsHoveredPanel")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIScrollSystem>());

		const pg::UUID panelID = pg::UUID::Generate();
		pg::ecs::Entity panelEnt = SeedPanel(panelID, -30.f);
		SeedScroll(-1.f);     // wheel down
		SeedHover(panelID);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ui::UIUpdateClipOffsetOneFrameComponent>();
		REQUIRE(view.size() == 1);
		CHECK(pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateClipOffsetOneFrameComponent>(panelEnt));
		// -30 + (-1) * 20 (scroll speed) = -50.
		CHECK(view.get<pg::ui::UIUpdateClipOffsetOneFrameComponent>(view.front()).m_ScrollOffset.y == Approx(-50.f));
	}

	// ---------------------------------------------------------------------------
	// Hovering a descendant of the panel (a clipped list item) still scrolls it.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIScrollSystem::ScrollsWhenHoveringDescendant")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIScrollSystem>());

		const pg::UUID panelID = pg::UUID::Generate();
		const pg::UUID childID = pg::UUID::Generate();
		pg::ecs::Entity panelEnt = SeedPanel(panelID, 0.f);

		pg::ecs::Registry& reg = pg::World::GetRegistryDirect();
		pg::ecs::Entity childEnt = reg.create();
		pg::ui::BaseComponent& childBase = reg.emplace<pg::ui::BaseComponent>(childEnt);
		childBase.m_UUID = childID;
		childBase.m_Parent = panelEnt;

		SeedScroll(-1.f);
		SeedHover(childID);   // hovering the child, not the panel directly

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ui::UIUpdateClipOffsetOneFrameComponent>();
		REQUIRE(view.size() == 1);
		CHECK(view.get<pg::ui::UIUpdateClipOffsetOneFrameComponent>(view.front()).m_ScrollOffset.y == Approx(-20.f));
	}

	// ---------------------------------------------------------------------------
	// Guard: no scroll event this frame -> no update.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIScrollSystem::NoScrollNoUpdate")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIScrollSystem>());

		const pg::UUID panelID = pg::UUID::Generate();
		SeedPanel(panelID, -30.f);
		SeedHover(panelID);   // hovered but no wheel

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ui::UIUpdateClipOffsetOneFrameComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: wheel but the pointer is not over the panel (or a descendant) -> no update.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIScrollSystem::NoUpdateWhenNotHovered")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIScrollSystem>());

		const pg::UUID panelID = pg::UUID::Generate();
		SeedPanel(panelID, -30.f);
		SeedScroll(-1.f);
		SeedHover(pg::UUID::Generate());   // hovering some unrelated element

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ui::UIUpdateClipOffsetOneFrameComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Edge: scrolling past the range is clamped (bottom limit).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIScrollSystem::ClampsScrollOffset")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::UIScrollSystem>());

		const pg::UUID panelID = pg::UUID::Generate();
		SeedPanel(panelID, -95.f);   // already near the bottom limit (-100)
		SeedScroll(-1.f);            // would go to -115, clamped to -100
		SeedHover(panelID);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ui::UIUpdateClipOffsetOneFrameComponent>();
		REQUIRE(view.size() == 1);
		CHECK(view.get<pg::ui::UIUpdateClipOffsetOneFrameComponent>(view.front()).m_ScrollOffset.y == Approx(-100.f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: declared sets contain the expected component types.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.UIScrollSystem::DeclareAccessIsCorrect")
	{
		sbx::UIScrollSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::MouseScrolledEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIClipComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIUpdateClipOffsetOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
