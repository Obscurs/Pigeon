#include "pch.h"
#include "Pigeon/Audio/AudioPlaybackSystem.h"

#include "Pigeon/Audio/AudioDevice.h"
#include "Pigeon/Audio/AudioDeviceSingletonComponent.h"
#include "Pigeon/Audio/AudioVolumeSingletonComponent.h"
#include "Pigeon/Audio/PauseAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/PlayAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/ResumeAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/SoundClip.h"
#include "Pigeon/Audio/StopAudioRequestOneFrameComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"

pg::SystemAccessDecl pg::AudioPlaybackSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::AudioDeviceSingletonComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::AudioVolumeSingletonComponent)),
		std::type_index(typeid(pg::PlayAudioRequestOneFrameComponent)),
		std::type_index(typeid(pg::StopAudioRequestOneFrameComponent)),
		std::type_index(typeid(pg::PauseAudioRequestOneFrameComponent)),
		std::type_index(typeid(pg::ResumeAudioRequestOneFrameComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::AudioDeviceSingletonComponent)),
	};
	return decl;
}

void pg::AudioPlaybackSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto deviceView = accessor.View<pg::AudioDeviceSingletonComponent>();
	if (deviceView.empty())
	{
		pg::AudioDeviceSingletonComponent component;
		component.m_Device = pg::AudioDevice::Create();
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<pg::AudioDeviceSingletonComponent>(ent, std::move(component));
		return;
	}

	const pg::S_Ptr<pg::AudioDevice>& devicePtr = deviceView.get<pg::AudioDeviceSingletonComponent>(deviceView.front()).m_Device;
	if (devicePtr == nullptr)
	{
		return;
	}
	pg::AudioDevice& device = *devicePtr;

	float master = 1.0f;
	float sound = 1.0f;
	float music = 1.0f;
	auto volumeView = accessor.View<const pg::AudioVolumeSingletonComponent>();
	if (!volumeView.empty())
	{
		const pg::AudioVolumeSingletonComponent& volume = volumeView.get<const pg::AudioVolumeSingletonComponent>(volumeView.front());
		master = volume.m_MasterVolume;
		sound = volume.m_SoundVolume;
		music = volume.m_MusicVolume;
	}
	device.SetCategoryVolumes(master, sound, music);

	for (pg::ecs::Entity ent : accessor.View<const pg::StopAudioRequestOneFrameComponent>())
	{
		device.Stop(accessor.Get<const pg::StopAudioRequestOneFrameComponent>(ent).m_VoiceID);
	}
	for (pg::ecs::Entity ent : accessor.View<const pg::PauseAudioRequestOneFrameComponent>())
	{
		device.Pause(accessor.Get<const pg::PauseAudioRequestOneFrameComponent>(ent).m_VoiceID);
	}
	for (pg::ecs::Entity ent : accessor.View<const pg::ResumeAudioRequestOneFrameComponent>())
	{
		device.Resume(accessor.Get<const pg::ResumeAudioRequestOneFrameComponent>(ent).m_VoiceID);
	}

	auto resourceView = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (!resourceView.empty())
	{
		const pg::ResourceMapSingletonComponent& resources = resourceView.get<const pg::ResourceMapSingletonComponent>(resourceView.front());
		for (pg::ecs::Entity ent : accessor.View<const pg::PlayAudioRequestOneFrameComponent>())
		{
			const pg::PlayAudioRequestOneFrameComponent& play = accessor.Get<const pg::PlayAudioRequestOneFrameComponent>(ent);
			const std::unordered_map<pg::UUID, pg::S_Ptr<pg::SoundClip>>::const_iterator it = resources.m_SoundMap.find(play.m_ClipID);
			if (it == resources.m_SoundMap.end())
			{
				continue;
			}
			pg::UUID voiceId = play.m_VoiceID;
			if (voiceId.IsNull())
			{
				voiceId = pg::UUID::Generate();
			}
			device.Play(voiceId, it->second, play.m_Category, play.m_Loop, play.m_Volume);
		}
	}

	device.Update();
}
