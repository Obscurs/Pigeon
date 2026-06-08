#include "Sandbox/AudioDebugPanelSystem.h"

#include "Pigeon/Audio/AudioVolumeSingletonComponent.h"
#include "Pigeon/Audio/SetAudioVolumeRequestOneFrameComponent.h"
#include "Pigeon/ECS/World.h"

#include <imgui.h>

pg::SystemAccessDecl sbx::AudioDebugPanelSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::AudioVolumeSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::SetAudioVolumeRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::AudioDebugPanelSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto volumeView = accessor.View<const pg::AudioVolumeSingletonComponent>();
	if (volumeView.empty())
	{
		return;
	}

	// The test build pushes no ImGuiLayer, so no ImGui context exists there; guard every call.
	if (ImGui::GetCurrentContext() == nullptr)
	{
		return;
	}

	const pg::AudioVolumeSingletonComponent& volume = volumeView.get<const pg::AudioVolumeSingletonComponent>(volumeView.front());
	float master = volume.m_MasterVolume;
	float sound = volume.m_SoundVolume;
	float music = volume.m_MusicVolume;

	ImGui::Begin("Audio");
	ImGui::Text("S: sound effect   M: music on/off   P: pause/resume");
	bool changed = false;
	changed |= ImGui::SliderFloat("Master volume", &master, 0.0f, 1.0f);
	changed |= ImGui::SliderFloat("Sound volume", &sound, 0.0f, 1.0f);
	changed |= ImGui::SliderFloat("Music volume", &music, 0.0f, 1.0f);
	ImGui::End();

	if (changed)
	{
		pg::SetAudioVolumeRequestOneFrameComponent request;
		request.m_MasterVolume = master;
		request.m_SoundVolume = sound;
		request.m_MusicVolume = music;
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceOneframe<pg::SetAudioVolumeRequestOneFrameComponent>(ent, std::move(request));
	}
}
