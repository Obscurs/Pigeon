#include "Sandbox/WindowDebugPanelSystem.h"

#include "Pigeon/Core/EWindowMode.h"
#include "Pigeon/Core/SetWindowResolutionRequestOneFrameComponent.h"
#include "Pigeon/Core/WindowConfigSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/WindowPanelSelectionSingletonComponent.h"

#include <imgui.h>

namespace
{
	struct ResolutionPreset
	{
		const char* m_Label;
		unsigned int m_Width;
		unsigned int m_Height;
	};

	const ResolutionPreset k_Presets[] = {
		{ "1280 x 720", 1280, 720 },
		{ "1600 x 900", 1600, 900 },
		{ "1920 x 1080", 1920, 1080 },
	};

	// Index of the preset matching the live resolution, or 0 when none matches.
	int FindPresetIndex(unsigned int width, unsigned int height)
	{
		for (int i = 0; i < IM_ARRAYSIZE(k_Presets); ++i)
		{
			if (k_Presets[i].m_Width == width && k_Presets[i].m_Height == height)
			{
				return i;
			}
		}
		return 0;
	}
}

pg::SystemAccessDecl sbx::WindowDebugPanelSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::WindowConfigSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::WindowPanelSelectionSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::WindowPanelSelectionSingletonComponent)),
		std::type_index(typeid(pg::SetWindowResolutionRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::WindowDebugPanelSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto windowConfigView = accessor.View<const pg::WindowConfigSingletonComponent>();
	if (windowConfigView.empty())
	{
		return;
	}
	const pg::WindowConfigSingletonComponent& windowConfig = windowConfigView.get<const pg::WindowConfigSingletonComponent>(windowConfigView.front());

	// Panel selection lives in ECS (like DebugPanelSystem's controls singleton). Seed it once from the
	// live config; thereafter it is owned by the user until Apply.
	auto selectionView = accessor.View<sbx::WindowPanelSelectionSingletonComponent>();
	if (selectionView.empty())
	{
		sbx::WindowPanelSelectionSingletonComponent selection;
		selection.m_PresetIndex = FindPresetIndex(windowConfig.m_Width, windowConfig.m_Height);
		selection.m_Mode = windowConfig.m_Mode;
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<sbx::WindowPanelSelectionSingletonComponent>(ent, std::move(selection));
		return;
	}

	// The test build pushes no ImGuiLayer, so no ImGui context exists there; guard every call.
	if (ImGui::GetCurrentContext() == nullptr)
	{
		return;
	}

	sbx::WindowPanelSelectionSingletonComponent& selection = selectionView.get<sbx::WindowPanelSelectionSingletonComponent>(selectionView.front());
	int presetIndex = selection.m_PresetIndex;
	int modeIndex = selection.m_Mode == pg::EWindowMode::eFullscreen ? 1 : 0;

	const char* presetLabels[] = { k_Presets[0].m_Label, k_Presets[1].m_Label, k_Presets[2].m_Label };

	ImGui::Begin("Window");
	ImGui::Text("Current: %u x %u (%s)", windowConfig.m_Width, windowConfig.m_Height,
		windowConfig.m_Mode == pg::EWindowMode::eFullscreen ? "fullscreen" : "windowed");
	ImGui::Combo("Resolution", &presetIndex, presetLabels, IM_ARRAYSIZE(presetLabels));
	ImGui::RadioButton("Windowed", &modeIndex, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Fullscreen", &modeIndex, 1);
	const bool apply = ImGui::Button("Apply");
	ImGui::End();

	selection.m_PresetIndex = presetIndex;
	selection.m_Mode = modeIndex == 1 ? pg::EWindowMode::eFullscreen : pg::EWindowMode::eWindowed;

	if (apply)
	{
		pg::SetWindowResolutionRequestOneFrameComponent request;
		request.m_Width = k_Presets[selection.m_PresetIndex].m_Width;
		request.m_Height = k_Presets[selection.m_PresetIndex].m_Height;
		request.m_Mode = selection.m_Mode;
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceOneframe<pg::SetWindowResolutionRequestOneFrameComponent>(ent, std::move(request));
	}
}
