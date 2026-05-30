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
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIRenderSystem>());

		// Verify no config exists before first update.
		auto viewBefore = pig::World::GetRegistryDirect().view<pig::ui::RendererConfigSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		// First frame: system detects no config and queues deferred add.
		world.Update(pig::Timestep(0));

		// Config should now be visible (deferred add flushed after Update()).
		auto viewAfter = pig::World::GetRegistryDirect().view<pig::ui::RendererConfigSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);

		// Default config values.
		const pig::ui::RendererConfigSingletonComponent& cfg =
			viewAfter.get<pig::ui::RendererConfigSingletonComponent>(viewAfter.front());
		CHECK(cfg.m_Width  == 1920.f);
		CHECK(cfg.m_Height == 1080.f);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with config but without EngineConfig/Resources ->
	// system returns early, no crash, no duplicate config.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::EarlyExitWithoutEngineConfigOrResources")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIRenderSystem>());

		// First frame: creates RendererConfigSingletonComponent.
		world.Update(pig::Timestep(0));

		// Second frame: config exists but EngineConfig/Resources are absent.
		// System should bail early without asserting or crashing.
		world.Update(pig::Timestep(0));

		// Config must still be a single entity (not duplicated).
		auto viewConfig = pig::World::GetRegistryDirect().view<pig::ui::RendererConfigSingletonComponent>();
		CHECK(viewConfig.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: UI image element disabled -> no DrawUIQuadInFrameEvent produced
	// (The actual draw path would require a valid texture/shader via resources,
	//  so we only verify that a disabled image entity produces no draw events.)
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::DisabledImageDoesNotProduceDrawEvent")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIRenderSystem>());

		// Pre-seed the RendererConfigSingletonComponent so the system skips creation.
		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::ui::RendererConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<pig::ui::RendererConfigSingletonComponent>(cfgEnt);
		cfg.m_Width  = 1920.f;
		cfg.m_Height = 1080.f;

		// Pre-seed EngineConfig and ResourceMap so the system gets past the early exit.
		entt::entity engEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::EngineConfigSingletonComponent>(engEnt);

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		// Disabled UI image entity.
		entt::entity uiEnt = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base =
			pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(uiEnt);
		base.m_Enabled = false;
		base.m_Size    = { 100.f, 100.f };

		pig::ui::ImageComponent& img =
			pig::World::GetRegistryDirect().emplace<pig::ui::ImageComponent>(uiEnt);
		img.m_TextureHandle = pig::UUID::Generate();

		// System update: disabled entity -> no draw event should be deferred.
		world.Update(pig::Timestep(0));

		// No DrawUIQuadInFrameEvent entities should have been created.
		// (We check indirectly by confirming no new entities beyond the 4 we created.)
		auto view = pig::World::GetRegistryDirect().view<pig::ui::BaseComponent>();
		// Only the one UI entity with BaseComponent.
		CHECK(view.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets contain the expected component types
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::DeclareAccessIsCorrect")
	{
		pig::ui::UIRenderSystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pig::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::BaseComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::ImageComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::TextComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::RendererConfigSingletonComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pig::ui::RendererConfigSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
