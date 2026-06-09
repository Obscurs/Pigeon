#pragma once
#include <nlohmann/json.hpp>

#include "Pigeon/Core/UUID.h"

namespace pg
{
	// One-frame request to update a save-data slot at runtime. Emitted by the app, applied (and persisted
	// to <savedataPath>/SaveSlots/<uuid>.json) by the engine SaveDataSystem. Must be an engine (pg) type
	// so the engine system can read it.
	struct SetSaveDataRequestOneFrameComponent
	{
		SetSaveDataRequestOneFrameComponent() = default;
		SetSaveDataRequestOneFrameComponent(const SetSaveDataRequestOneFrameComponent&) = default;

		pg::UUID m_UUID;
		nlohmann::json m_Json;
	};
}
