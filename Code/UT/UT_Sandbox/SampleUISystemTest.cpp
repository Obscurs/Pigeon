#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/OrthographicCameraComponent.h>
#include <Pigeon/Core/ResourceMapSingletonComponent.h>
#include <Pigeon/Renderer/DrawQuadInFrameEvent.h>
#include <Pigeon/Renderer/DrawSpriteInFrameEvent.h>
#include <Pigeon/Renderer/DrawStringInFrameEvent.h>
#include <Pigeon/UI/UIComponents.h>
#include "Sandbox/SampleUISystem.h"
#include "Sandbox/SampleUIConfigSingletonComponent.h"
#include "Sandbox/SampleUISingletonComponent.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no ResourceMapSingletonComponent -> system returns early, no component created
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::NoOpWithoutResourceMap")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		// Provide config but NOT resource map.
		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<sbx::SampleUIConfigSingletonComponent>(cfgEnt);

		world.Update(pig::Timestep(0));

		auto view = pig::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no SampleUIConfigSingletonComponent -> system returns early, no component created
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::NoOpWithoutConfig")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		// Provide resources but NOT config.
		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		world.Update(pig::Timestep(0));

		auto view = pig::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: first Update() with ResourceMap and Config present ->
	// system creates SampleUISingletonComponent and OrthographicCameraComponent
	// via deferred add. Both visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::CreatesComponentsOnFirstFrame")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<sbx::SampleUIConfigSingletonComponent>(cfgEnt);

		auto viewBefore = pig::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pig::Timestep(0));

		auto viewAfter = pig::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: created SampleUISingletonComponent has FontID copied from config
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::CreatedComponentHasFontIDFromConfig")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		sbx::SampleUIConfigSingletonComponent& cfg =
			pig::World::GetRegistryDirect().emplace<sbx::SampleUIConfigSingletonComponent>(cfgEnt);
		cfg.m_DefaultFontID = pig::UUID::Generate();

		world.Update(pig::Timestep(0));

		auto view = pig::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		REQUIRE(view.size() == 1);

		const sbx::SampleUISingletonComponent& comp =
			view.get<sbx::SampleUISingletonComponent>(view.front());
		CHECK(comp.m_FontID == cfg.m_DefaultFontID);
	}

	// ---------------------------------------------------------------------------
	// Happy path: first Update() also creates an OrthographicCameraComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::CreatesOrthographicCameraOnFirstFrame")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<sbx::SampleUIConfigSingletonComponent>(cfgEnt);

		auto viewBefore = pig::World::GetRegistryDirect().view<pig::OrthographicCameraComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pig::Timestep(0));

		auto viewAfter = pig::World::GetRegistryDirect().view<pig::OrthographicCameraComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::DeclareAccessIsCorrect")
	{
		sbx::SampleUISystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::SampleUISingletonComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pig::OrthographicCameraComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pig::ui::LoadLayoutEvent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::SampleUISingletonComponent))) > 0);

		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pig::DrawQuadInFrameEvent))) > 0);
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pig::DrawSpriteInFrameEvent))) > 0);
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pig::DrawStringInFrameEvent))) > 0);

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SampleUIConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::BaseComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::ImageComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::TextComponent))) > 0);
	}

} // namespace CatchTestsetFail
