#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Audio/SoundClip.h"
#include "Pigeon/Core/ResourceManagerSystem.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no ResourceMapSingletonComponent ->
	// system creates one via deferred add. Visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::CreatesResourceMapOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		auto viewBefore = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with ResourceMapSingletonComponent already present ->
	// system does nothing (no duplicate created)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::DoesNotDuplicateWhenMapExists")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		// First frame: the system itself creates the ResourceMapSingletonComponent
		// (the test must not pre-create a component the system adds).
		world.Update(pg::Timestep(0));

		// Second frame: the map already exists, so the guard fires and no duplicate is added.
		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has a default texture entry
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasDefaultTexture")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		// Default texture must always be present in the texture map.
		CHECK(map.m_TextureMap.find(map.m_DefaultTexture) != map.m_TextureMap.end());
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has at least one shader entry
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasShaders")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		CHECK(!map.m_ShaderMap.empty());
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has at least one sound clip entry
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasSounds")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		REQUIRE(!map.m_SoundMap.empty());
		// Every loaded clip exposes its resolved path.
		for (const std::pair<const pg::UUID, pg::S_Ptr<pg::SoundClip>>& entry : map.m_SoundMap)
		{
			CHECK(entry.second != nullptr);
			CHECK(!entry.second->GetPath().empty());
		}
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has the JSON asset declared in the manifest,
	// keyed by its UUID and holding the parsed file content.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasJSONAsset")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		REQUIRE(!map.m_JSONMap.empty());

		std::unordered_map<pg::UUID, nlohmann::json>::const_iterator it =
			map.m_JSONMap.find(pg::UUID("d4000000-0000-4000-8000-000000000001"));
		REQUIRE(it != map.m_JSONMap.end());

		const nlohmann::json& asset = it->second;
		CHECK(asset["name"].get<std::string>() == "test-asset");
		CHECK(asset["value"].get<int>() == 42);
		CHECK(asset["nested"]["flag"].get<bool>() == true);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::DeclareAccessIsCorrect")
	{
		pg::ResourceManagerSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
