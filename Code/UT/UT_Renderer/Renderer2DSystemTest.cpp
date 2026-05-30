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
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::Renderer2DSystem>());

		// Provide resources and config but NOT a camera.
		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::EngineConfigSingletonComponent>(cfgEnt);

		world.Update(pig::Timestep(0));

		// No RendererDataSingletonComponent should have been created.
		auto view = pig::World::GetRegistryDirect().view<pig::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no resources -> system returns early, no crash
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::NoOpWithoutResourceMap")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::Renderer2DSystem>());

		// Provide camera and config but NOT resources.
		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);

		entt::entity cfgEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::EngineConfigSingletonComponent>(cfgEnt);

		world.Update(pig::Timestep(0));

		auto view = pig::World::GetRegistryDirect().view<pig::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no engine config -> system returns early, no crash
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::NoOpWithoutEngineConfig")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::Renderer2DSystem>());

		// Provide camera and resources but NOT engine config.
		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		world.Update(pig::Timestep(0));

		auto view = pig::World::GetRegistryDirect().view<pig::RendererDataSingletonComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.Renderer2DSystem::DeclareAccessIsCorrect")
	{
		pig::Renderer2DSystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pig::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::OrthographicCameraComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::DrawQuadInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::DrawSpriteInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::DrawStringInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::DrawUIQuadInFrameEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::DrawUIStringInFrameEvent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pig::RendererDataSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pig::RendererDataSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
