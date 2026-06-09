#include "pch.h"
#include "Pigeon/Core/SaveDataSystem.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/FileUtils.h"
#include "Pigeon/Core/SaveDataSingletonComponent.h"
#include "Pigeon/Core/SetSaveDataRequestOneFrameComponent.h"
#include "Pigeon/ECS/World.h"

namespace
{
	std::string SlotsDir(const std::string& savedataPath)
	{
		return savedataPath + "/SaveSlots";
	}

	std::string SlotFilePath(const std::string& savedataPath, const pg::UUID& uuid)
	{
		return SlotsDir(savedataPath) + "/" + uuid.ToString() + ".json";
	}

	// Loads every <savedataPath>/SaveSlots/<uuid>.json into the map. The filename stem is the slot UUID.
	// An empty savedata path (e.g. the Testing build) yields an empty map.
	void LoadSlots(pg::SaveDataSingletonComponent& component, const std::string& savedataPath)
	{
		if (savedataPath.empty())
		{
			return;
		}

		for (const std::string& filePath : pg::ListFilesWithExtension(SlotsDir(savedataPath), ".json"))
		{
			std::string stem = pg::GetFileStem(filePath);
			pg::UUID uuid(stem);
			component.m_SaveDataMap[uuid] = json::parse(pg::ReadFileToString(filePath));
		}
	}

	// Writes the slot's JSON to <savedataPath>/SaveSlots/<uuid>.json, creating the folder when missing.
	// No-op when the savedata path is empty.
	void PersistSlot(const std::string& savedataPath, const pg::UUID& uuid, const json& document)
	{
		if (savedataPath.empty())
		{
			return;
		}

		pg::EnsureDirectoryExists(SlotsDir(savedataPath));
		pg::WriteStringToFile(SlotFilePath(savedataPath, uuid), document.dump(1, '\t') + "\n");
	}
}

pg::SystemAccessDecl pg::SaveDataSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
		std::type_index(typeid(pg::SetSaveDataRequestOneFrameComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::SaveDataSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::SaveDataSingletonComponent)),
	};
	return decl;
}

void pg::SaveDataSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto saveDataView = accessor.View<pg::SaveDataSingletonComponent>();
	if (saveDataView.empty())
	{
		auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
		if (configView.empty())
		{
			return;
		}
		const pg::EngineConfigSingletonComponent& config = configView.get<const pg::EngineConfigSingletonComponent>(configView.front());

		pg::SaveDataSingletonComponent saveData;
		LoadSlots(saveData, config.m_SavedataPath);

		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<pg::SaveDataSingletonComponent>(ent, std::move(saveData));
		return;
	}

	pg::SaveDataSingletonComponent& saveData = saveDataView.get<pg::SaveDataSingletonComponent>(saveDataView.front());

	auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
	const std::string savedataPath = configView.empty() ? std::string() :
		configView.get<const pg::EngineConfigSingletonComponent>(configView.front()).m_SavedataPath;

	// Apply each save request to both the loaded resource (the map) and the data resource (the file).
	for (pg::ecs::Entity ent : accessor.View<const pg::SetSaveDataRequestOneFrameComponent>())
	{
		const pg::SetSaveDataRequestOneFrameComponent& request = accessor.Get<const pg::SetSaveDataRequestOneFrameComponent>(ent);
		saveData.m_SaveDataMap[request.m_UUID] = request.m_Json;
		PersistSlot(savedataPath, request.m_UUID, request.m_Json);
	}
}
