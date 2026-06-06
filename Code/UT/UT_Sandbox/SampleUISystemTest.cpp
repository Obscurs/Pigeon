#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Pigeon/Renderer/DrawStringInFrameEvent.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SampleUIConfigSingletonComponent.h"
#include "Sandbox/SampleUISingletonComponent.h"
#include "Sandbox/SampleUISystem.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no ResourceMapSingletonComponent -> system returns early, no component created
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::NoOpWithoutResourceMap")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		// Provide config but NOT resource map.
		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::SampleUIConfigSingletonComponent>(cfgEnt);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no SampleUIConfigSingletonComponent -> system returns early, no component created
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::NoOpWithoutConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		// Provide resources but NOT config.
		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: first Update() with ResourceMap and Config present ->
	// system creates SampleUISingletonComponent and OrthographicCameraComponent
	// via deferred add. Both visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::CreatesComponentsOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::SampleUIConfigSingletonComponent>(cfgEnt);

		auto viewBefore = pg::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: created SampleUISingletonComponent has FontID copied from config
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::CreatedComponentHasFontIDFromConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		sbx::SampleUIConfigSingletonComponent& cfg =
			pg::World::GetRegistryDirect().emplace<sbx::SampleUIConfigSingletonComponent>(cfgEnt);
		cfg.m_DefaultFontID = pg::UUID::Generate();

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<sbx::SampleUISingletonComponent>();
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
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SampleUISystem>());

		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		pg::ecs::Entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::SampleUIConfigSingletonComponent>(cfgEnt);

		auto viewBefore = pg::World::GetRegistryDirect().view<pg::OrthographicCameraComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::OrthographicCameraComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SampleUISystem::DeclareAccessIsCorrect")
	{
		sbx::SampleUISystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::SampleUISingletonComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::OrthographicCameraComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::LoadLayoutEvent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::SampleUISingletonComponent))) > 0);

		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawQuadInFrameEvent))) > 0);
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawSpriteInFrameEvent))) > 0);
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawStringInFrameEvent))) > 0);

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SampleUIConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::ImageComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::TextComponent))) > 0);
	}

} // namespace CatchTestsetFail
