#include "pch.h"
#include "Pigeon/Audio/AudioVolumeSystem.h"

#include "Pigeon/Audio/AudioVolumeSingletonComponent.h"
#include "Pigeon/Audio/SetAudioVolumeRequestOneFrameComponent.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/ECS/World.h"

pg::SystemAccessDecl pg::AudioVolumeSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
		std::type_index(typeid(pg::SetAudioVolumeRequestOneFrameComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::AudioVolumeSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::AudioVolumeSingletonComponent)),
	};
	return decl;
}

void pg::AudioVolumeSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto volumeView = accessor.View<pg::AudioVolumeSingletonComponent>();
	if (volumeView.empty())
	{
		auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
		if (configView.empty())
		{
			return;
		}
		const pg::EngineConfigSingletonComponent& config = configView.get<const pg::EngineConfigSingletonComponent>(configView.front());

		pg::AudioVolumeSingletonComponent volume;
		volume.m_MasterVolume = config.m_MasterVolume;
		volume.m_SoundVolume = config.m_SoundVolume;
		volume.m_MusicVolume = config.m_MusicVolume;

		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<pg::AudioVolumeSingletonComponent>(ent, std::move(volume));
		return;
	}

	pg::AudioVolumeSingletonComponent& volume = volumeView.get<pg::AudioVolumeSingletonComponent>(volumeView.front());

	// Apply runtime volume changes; the last request of the frame wins.
	for (pg::ecs::Entity ent : accessor.View<const pg::SetAudioVolumeRequestOneFrameComponent>())
	{
		const pg::SetAudioVolumeRequestOneFrameComponent& request = accessor.Get<const pg::SetAudioVolumeRequestOneFrameComponent>(ent);
		volume.m_MasterVolume = request.m_MasterVolume;
		volume.m_SoundVolume = request.m_SoundVolume;
		volume.m_MusicVolume = request.m_MusicVolume;
	}
}
