#include "Sandbox/SaveDataDebugPanelSystem.h"

#include <cstring>

#include "Pigeon/Core/SaveDataSingletonComponent.h"
#include "Pigeon/Core/SetSaveDataRequestOneFrameComponent.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/SaveDataPanelStateSingletonComponent.h"

#include <imgui.h>

namespace
{
	// Copies a string into the fixed edit buffer, always null-terminated and never overflowing.
	void SetBuffer(std::array<char, 2048>& buffer, const std::string& text)
	{
		const size_t count = text.size() < buffer.size() - 1 ? text.size() : buffer.size() - 1;
		std::memcpy(buffer.data(), text.data(), count);
		buffer[count] = '\0';
	}

	// Refreshes the edit buffer from the selected slot's JSON, or an empty object when the slot is unknown.
	void LoadSelectedIntoBuffer(sbx::SaveDataPanelStateSingletonComponent& state, const pg::SaveDataSingletonComponent& saveData)
	{
		std::unordered_map<pg::UUID, json>::const_iterator it = saveData.m_SaveDataMap.find(state.m_SelectedSlot);
		if (it != saveData.m_SaveDataMap.end())
		{
			SetBuffer(state.m_EditBuffer, it->second.dump(1, '\t'));
		}
		else
		{
			SetBuffer(state.m_EditBuffer, "{}");
		}
	}
}

pg::SystemAccessDecl sbx::SaveDataDebugPanelSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::SaveDataSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::SaveDataPanelStateSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::SaveDataPanelStateSingletonComponent)),
		std::type_index(typeid(pg::SetSaveDataRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::SaveDataDebugPanelSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto saveDataView = accessor.View<const pg::SaveDataSingletonComponent>();
	if (saveDataView.empty())
	{
		return;
	}
	const pg::SaveDataSingletonComponent& saveData = saveDataView.get<const pg::SaveDataSingletonComponent>(saveDataView.front());

	// Seed the panel state once: select the first slot (if any) and load its JSON into the buffer.
	auto stateView = accessor.View<sbx::SaveDataPanelStateSingletonComponent>();
	if (stateView.empty())
	{
		sbx::SaveDataPanelStateSingletonComponent state;
		if (!saveData.m_SaveDataMap.empty())
		{
			state.m_SelectedSlot = saveData.m_SaveDataMap.begin()->first;
		}
		LoadSelectedIntoBuffer(state, saveData);
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<sbx::SaveDataPanelStateSingletonComponent>(ent, std::move(state));
		return;
	}

	// The test build pushes no ImGuiLayer, so no ImGui context exists there; guard every call.
	if (ImGui::GetCurrentContext() == nullptr)
	{
		return;
	}

	sbx::SaveDataPanelStateSingletonComponent& state = stateView.get<sbx::SaveDataPanelStateSingletonComponent>(stateView.front());

	ImGui::Begin("Save Data");

	ImGui::Text("Slots:");
	for (const std::pair<const pg::UUID, json>& slot : saveData.m_SaveDataMap)
	{
		const std::string label = slot.first.ToString();
		const bool selected = slot.first == state.m_SelectedSlot;
		if (ImGui::Selectable(label.c_str(), selected))
		{
			state.m_SelectedSlot = slot.first;
			LoadSelectedIntoBuffer(state, saveData);
		}
	}

	if (ImGui::Button("New slot"))
	{
		state.m_SelectedSlot = pg::UUID::Generate();
		SetBuffer(state.m_EditBuffer, "{}");
	}

	ImGui::Separator();
	ImGui::Text("Selected: %s", state.m_SelectedSlot.ToString().c_str());
	ImGui::InputTextMultiline("##json", state.m_EditBuffer.data(), state.m_EditBuffer.size());

	const bool load = ImGui::Button("Load");
	ImGui::SameLine();
	const bool save = ImGui::Button("Save");
	ImGui::End();

	if (load)
	{
		LoadSelectedIntoBuffer(state, saveData);
	}

	if (save && !state.m_SelectedSlot.IsNull())
	{
		// Non-throwing parse: a malformed buffer yields a discarded value, which we simply ignore.
		const json parsed = json::parse(state.m_EditBuffer.data(), nullptr, false);
		if (!parsed.is_discarded())
		{
			pg::SetSaveDataRequestOneFrameComponent request;
			request.m_UUID = state.m_SelectedSlot;
			request.m_Json = parsed;
			pg::ecs::Entity ent = accessor.Create();
			accessor.EmplaceOneframe<pg::SetSaveDataRequestOneFrameComponent>(ent, std::move(request));
		}
	}
}
