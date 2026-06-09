#pragma once
#include <nlohmann/json.hpp>
#include <unordered_map>

#include "Pigeon/Core/UUID.h"

namespace pg
{
	// Live save-data resource: one JSON document per save slot, keyed by the slot UUID. Loaded from
	// <savedataPath>/SaveSlots/<uuid>.json at startup and updated in place by save requests. Owned
	// (added + written) solely by SaveDataSystem.
	struct SaveDataSingletonComponent
	{
		SaveDataSingletonComponent() {};
		SaveDataSingletonComponent(const SaveDataSingletonComponent&) = default;

		std::unordered_map<pg::UUID, nlohmann::json> m_SaveDataMap;
	};
}
