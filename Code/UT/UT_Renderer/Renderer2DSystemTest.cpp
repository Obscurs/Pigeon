#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/EngineConfigSingletonComponent.h>
#include <Pigeon/Core/OrthographicCameraComponent.h>
#include <Pigeon/Core/ResourceMapSingletonComponent.h>
#include <Pigeon/Renderer/Renderer2DSystem.h>
#include <Pigeon/Renderer/RendererDataSingletonComponent.h>
#include <Pigeon/Renderer/DrawQuadInFrameEvent.h>
#include <Pigeon/Renderer/DrawSpriteInFrameEvent.h>
#include <Pigeon/Renderer/DrawStringInFrameEvent.h>
#include <Pigeon/Renderer/DrawUIQuadInFrameEvent.h>
#include <Pigeon/Renderer/DrawUIStringInFrameEvent.h>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no camera -> system returns early, no crash, no renderer data created
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::NoOpWithoutCamera")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());

		// Provide resources and config but NOT a camera.
		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		entt::entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(cfgEnt);

		world.Update(pg::Timestep(0));

		// No RendererDataSingletonComponent should have been created.
		auto view = pg::World::GetRegistryDirect().view<pg::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no resources -> system returns early, no crash
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::NoOpWithoutResourceMap")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());

		// Provide camera and config but NOT resources.
		entt::entity camEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);

		entt::entity cfgEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(cfgEnt);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no engine config -> system returns early, no crash
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::NoOpWithoutEngineConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::Renderer2DSystem>());

		// Provide camera and resources but NOT engine config.
		entt::entity camEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::DeclareAccessIsCorrect")
	{
		pg::Renderer2DSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::OrthographicCameraComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawQuadInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawSpriteInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawStringInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawUIQuadInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::DrawUIStringInFrameEvent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::RendererDataSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::RendererDataSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
