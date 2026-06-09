#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/SaveDataSingletonComponent.h"
#include "Pigeon/Core/SaveDataSystem.h"
#include "Pigeon/Core/SetSaveDataRequestOneFrameComponent.h"
#include "Pigeon/Core/UUID.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"

namespace CatchTestsetFail
{
	namespace
	{
		const char* k_SlotA = "11111111-1111-1111-1111-111111111111";
		const char* k_SlotB = "22222222-2222-2222-2222-222222222222";

		// A unique savedata directory per invocation so tests never collide on disk. The system reads
		// and writes <savedataPath>/SaveSlots, so the SaveSlots subfolder is what we seed.
		std::string MakeSavedataDir(const std::string& tag)
		{
			const std::string dir = "./SaveDataTest_" + tag;
			std::filesystem::remove_all(dir);
			std::filesystem::create_directories(dir + "/SaveSlots");
			return dir;
		}

		void WriteSlotFile(const std::string& savedataDir, const std::string& uuid, const std::string& content)
		{
			std::ofstream file(savedataDir + "/SaveSlots/" + uuid + ".json");
			REQUIRE(file.is_open());
			file << content;
		}

		json ReadSlotFile(const std::string& savedataDir, const std::string& uuid)
		{
			std::ifstream file(savedataDir + "/SaveSlots/" + uuid + ".json");
			REQUIRE(file.is_open());
			std::ostringstream ss;
			ss << file.rdbuf();
			return json::parse(ss.str());
		}

		pg::EngineConfigSingletonComponent MakeConfig(const std::string& savedataPath)
		{
			pg::EngineConfigSingletonComponent config;
			config.m_SavedataPath = savedataPath;
			return config;
		}

		const pg::SaveDataSingletonComponent& GetSaveData()
		{
			auto view = pg::World::GetRegistryDirect().view<pg::SaveDataSingletonComponent>();
			REQUIRE(view.size() == 1);
			return view.get<pg::SaveDataSingletonComponent>(view.front());
		}
	}

	// Happy path: with the engine config present, the system loads every <uuid>.json under SaveSlots
	// into the save-data map, keyed by the filename UUID.
	TEST_CASE("Core.SaveDataSystem::LoadsSlotsFromSaveSlotsFolder")
	{
		const std::string savedataDir = MakeSavedataDir("Loads");
		WriteSlotFile(savedataDir, k_SlotA, "{\n\t\"counter\": 7\n}\n");
		WriteSlotFile(savedataDir, k_SlotB, "{\n\t\"name\": \"hero\"\n}\n");

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SaveDataSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(savedataDir));

		world.Update(pg::Timestep(0));

		const pg::SaveDataSingletonComponent& saveData = GetSaveData();
		CHECK(saveData.m_SaveDataMap.size() == 2);
		REQUIRE(saveData.m_SaveDataMap.count(pg::UUID(k_SlotA)) == 1);
		REQUIRE(saveData.m_SaveDataMap.count(pg::UUID(k_SlotB)) == 1);
		CHECK(saveData.m_SaveDataMap.at(pg::UUID(k_SlotA))["counter"].get<int>() == 7);
		CHECK(saveData.m_SaveDataMap.at(pg::UUID(k_SlotB))["name"].get<std::string>() == "hero");

		std::filesystem::remove_all(savedataDir);
	}

	// Guard: with no engine config the system creates nothing (it does not know where to load from).
	TEST_CASE("Core.SaveDataSystem::NoConfigCreatesNothing")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SaveDataSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::SaveDataSingletonComponent>().size() == 0);
	}

	// Edge: an empty savedata path still produces the singleton (empty map) so requests can land.
	TEST_CASE("Core.SaveDataSystem::EmptySavedataPathCreatesEmptySingleton")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SaveDataSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(""));

		world.Update(pg::Timestep(0));

		const pg::SaveDataSingletonComponent& saveData = GetSaveData();
		CHECK(saveData.m_SaveDataMap.empty());
	}

	// Guard: the singleton is created once and not rebuilt on later frames.
	TEST_CASE("Core.SaveDataSystem::DoesNotDuplicateSingleton")
	{
		const std::string savedataDir = MakeSavedataDir("Dup");
		WriteSlotFile(savedataDir, k_SlotA, "{\n\t\"counter\": 1\n}\n");

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SaveDataSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(savedataDir));

		world.Update(pg::Timestep(0));
		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::SaveDataSingletonComponent>().size() == 1);

		std::filesystem::remove_all(savedataDir);
	}

	// Happy path: a save request updates the loaded (in-memory) slot entry in place.
	TEST_CASE("Core.SaveDataSystem::SaveRequestUpdatesLoadedResource")
	{
		const std::string savedataDir = MakeSavedataDir("UpdateMem");
		WriteSlotFile(savedataDir, k_SlotA, "{\n\t\"counter\": 1\n}\n");

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SaveDataSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(savedataDir));

		world.Update(pg::Timestep(0)); // loads the singleton

		pg::SetSaveDataRequestOneFrameComponent request;
		request.m_UUID = pg::UUID(k_SlotA);
		request.m_Json["counter"] = 42;
		pg::ecs::Entity reqEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SetSaveDataRequestOneFrameComponent>(reqEnt, request);

		world.Update(pg::Timestep(0)); // applies the request

		const pg::SaveDataSingletonComponent& saveData = GetSaveData();
		REQUIRE(saveData.m_SaveDataMap.count(pg::UUID(k_SlotA)) == 1);
		CHECK(saveData.m_SaveDataMap.at(pg::UUID(k_SlotA))["counter"].get<int>() == 42);

		std::filesystem::remove_all(savedataDir);
	}

	// A save request for a brand-new slot adds it to the loaded map.
	TEST_CASE("Core.SaveDataSystem::SaveRequestAddsNewSlot")
	{
		const std::string savedataDir = MakeSavedataDir("NewSlot");

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SaveDataSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(savedataDir));

		world.Update(pg::Timestep(0)); // loads an empty singleton

		pg::SetSaveDataRequestOneFrameComponent request;
		request.m_UUID = pg::UUID(k_SlotB);
		request.m_Json["name"] = "new";
		pg::ecs::Entity reqEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SetSaveDataRequestOneFrameComponent>(reqEnt, request);

		world.Update(pg::Timestep(0));

		const pg::SaveDataSingletonComponent& saveData = GetSaveData();
		REQUIRE(saveData.m_SaveDataMap.count(pg::UUID(k_SlotB)) == 1);
		CHECK(saveData.m_SaveDataMap.at(pg::UUID(k_SlotB))["name"].get<std::string>() == "new");

		std::filesystem::remove_all(savedataDir);
	}

	// Persistence: a save request rewrites the slot's <uuid>.json on disk with the new JSON.
	TEST_CASE("Core.SaveDataSystem::SaveRequestPersistsToDisk")
	{
		const std::string savedataDir = MakeSavedataDir("Persist");
		WriteSlotFile(savedataDir, k_SlotA, "{\n\t\"counter\": 1\n}\n");

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SaveDataSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(savedataDir));

		world.Update(pg::Timestep(0));

		pg::SetSaveDataRequestOneFrameComponent request;
		request.m_UUID = pg::UUID(k_SlotA);
		request.m_Json["counter"] = 99;
		pg::ecs::Entity reqEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SetSaveDataRequestOneFrameComponent>(reqEnt, request);

		world.Update(pg::Timestep(0));

		const json saved = ReadSlotFile(savedataDir, k_SlotA);
		REQUIRE(saved.contains("counter"));
		CHECK(saved["counter"].get<int>() == 99);

		std::filesystem::remove_all(savedataDir);
	}

	// A save request for a new slot writes a new <uuid>.json file on disk.
	TEST_CASE("Core.SaveDataSystem::SaveRequestWritesNewFile")
	{
		const std::string savedataDir = MakeSavedataDir("WriteNew");

		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SaveDataSystem>());

		pg::ecs::Entity configEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(configEnt, MakeConfig(savedataDir));

		world.Update(pg::Timestep(0));

		pg::SetSaveDataRequestOneFrameComponent request;
		request.m_UUID = pg::UUID(k_SlotB);
		request.m_Json["level"] = 3;
		pg::ecs::Entity reqEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::SetSaveDataRequestOneFrameComponent>(reqEnt, request);

		world.Update(pg::Timestep(0));

		CHECK(std::filesystem::exists(savedataDir + "/SaveSlots/" + k_SlotB + ".json"));
		const json saved = ReadSlotFile(savedataDir, k_SlotB);
		CHECK(saved["level"].get<int>() == 3);

		std::filesystem::remove_all(savedataDir);
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Core.SaveDataSystem::DeclareAccessIsCorrect")
	{
		pg::SaveDataSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::SetSaveDataRequestOneFrameComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::SaveDataSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::SaveDataSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
