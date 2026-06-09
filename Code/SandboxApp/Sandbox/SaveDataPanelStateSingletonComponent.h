#pragma once
#include <array>

#include "Pigeon/Core/UUID.h"

namespace sbx
{
	// Panel-local state for the save-data debug widget: which slot is selected and the editable JSON
	// text shown for it. Lives in ECS like the other debug panels' selection singletons.
	struct SaveDataPanelStateSingletonComponent
	{
		SaveDataPanelStateSingletonComponent() = default;
		SaveDataPanelStateSingletonComponent(const SaveDataPanelStateSingletonComponent&) = default;

		pg::UUID m_SelectedSlot;
		std::array<char, 2048> m_EditBuffer{};
	};
}
