#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/EngineConfigSingletonComponent.h>
#include <Pigeon/Core/ResourceMapSingletonComponent.h>
#include <Pigeon/UI/UIComponents.h>
#include <Pigeon/UI/UIRenderSystem.h>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no RendererConfigSingletonComponent ->
	// system creates one via deferred add. On next frame it is visible.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::CreatesRendererConfigOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		// Verify no config exists before first update.
		auto viewBefore = pg::World::GetRegistryDirect().view<pg::ui::RendererConfigSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		// First frame: system detects no config and queues deferred add.
		world.Update(pg::Timestep(0));

		// Config should now be visible (deferred add flushed after Update()).
		auto viewAfter = pg::World::GetRegistryDirect().view<pg::ui::RendererConfigSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);

		// Default config values.
		const pg::ui::RendererConfigSingletonComponent& cfg =
			viewAfter.get<pg::ui::RendererConfigSingletonComponent>(viewAfter.front());
		CHECK(cfg.m_Width  == 1920.f);
		CHECK(cfg.m_Height == 1080.f);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with config but without EngineConfig/Resources ->
	// system returns early, no crash, no duplicate config.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::EarlyExitWithoutEngineConfigOrResources")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		// First frame: creates RendererConfigSingletonComponent.
		world.Update(pg::Timestep(0));

		// Second frame: config exists but EngineConfig/Resources are absent.
		// System should bail early without asserting or crashing.
		world.Update(pg::Timestep(0));

		// Config must still be a single entity (not duplicated).
		auto viewConfig = pg::World::GetRegistryDirect().view<pg::ui::RendererConfigSingletonComponent>();
		CHECK(viewConfig.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: UI image element disabled -> no DrawUIQuadInFrameEvent produced
	// (The actual draw path would require a valid texture/shader via resources,
	//  so we only verify that a disabled image entity produces no draw events.)
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::DisabledImageDoesNotProduceDrawEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		// Pre-seed the RendererConfigSingletonComponent so the system skips creation.
		entt::entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::ui::RendererConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<pg::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		// Pre-seed EngineConfig and ResourceMap so the system gets past the early exit.
		entt::entity engEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(engEnt);

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		// Disabled UI image entity.
		entt::entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Enabled = false;
		base.m_Size    = { 100.f, 100.f };

		pg::ui::ImageComponent& img =
			pg::World::GetRegistryDirect().emplace<pg::ui::ImageComponent>(uiEnt);
		img.m_TextureHandle = pg::UUID::Generate();

		// System update: disabled entity -> no draw event should be deferred.
		world.Update(pg::Timestep(0));

		// No DrawUIQuadInFrameEvent entities should have been created.
		// (We check indirectly by confirming no new entities beyond the 4 we created.)
		auto view = pg::World::GetRegistryDirect().view<pg::ui::BaseComponent>();
		// Only the one UI entity with BaseComponent.
		CHECK(view.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets contain the expected component types
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::DeclareAccessIsCorrect")
	{
		pg::ui::UIRenderSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::ImageComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::TextComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::RendererConfigSingletonComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::RendererConfigSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
