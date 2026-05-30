#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/ResourceManagerSystem.h>
#include <Pigeon/Core/ResourceMapSingletonComponent.h>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no ResourceMapSingletonComponent ->
	// system creates one via deferred add. Visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::CreatesResourceMapOnFirstFrame")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ResourceManagerSystem>());

		auto viewBefore = pig::World::GetRegistryDirect().view<pig::ResourceMapSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pig::Timestep(0));

		auto viewAfter = pig::World::GetRegistryDirect().view<pig::ResourceMapSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with ResourceMapSingletonComponent already present ->
	// system does nothing (no duplicate created)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::DoesNotDuplicateWhenMapExists")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ResourceManagerSystem>());

		// Pre-seed a ResourceMapSingletonComponent so the system guard fires.
		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(ent);

		world.Update(pig::Timestep(0));

		auto viewAfter = pig::World::GetRegistryDirect().view<pig::ResourceMapSingletonComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has a default texture entry
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasDefaultTexture")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ResourceManagerSystem>());

		world.Update(pig::Timestep(0));

		auto view = pig::World::GetRegistryDirect().view<pig::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pig::ResourceMapSingletonComponent& map =
			view.get<pig::ResourceMapSingletonComponent>(view.front());

		// Default texture must always be present in the texture map.
		CHECK(map.m_TextureMap.find(map.m_DefaultTexture) != map.m_TextureMap.end());
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has at least one shader entry
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasShaders")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ResourceManagerSystem>());

		world.Update(pig::Timestep(0));

		auto view = pig::World::GetRegistryDirect().view<pig::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pig::ResourceMapSingletonComponent& map =
			view.get<pig::ResourceMapSingletonComponent>(view.front());

		CHECK(!map.m_ShaderMap.empty());
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::DeclareAccessIsCorrect")
	{
		pig::ResourceManagerSystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pig::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pig::ResourceMapSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
