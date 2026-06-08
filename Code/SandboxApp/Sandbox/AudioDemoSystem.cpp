#include "Sandbox/AudioDemoSystem.h"

#include "Pigeon/Audio/EAudioCategory.h"
#include "Pigeon/Audio/PauseAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/PlayAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/ResumeAudioRequestOneFrameComponent.h"
#include "Pigeon/Audio/StopAudioRequestOneFrameComponent.h"
#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/AudioDemoStateSingletonComponent.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"

namespace
{
	bool WasKeyJustPressed(const pg::InputStateSingletonComponent& input, int key)
	{
		const std::unordered_map<int, int>::const_iterator it = input.m_KeysPressed.find(key);
		return it != input.m_KeysPressed.end() && it->second == 1;
	}

	void EmitPlay(pg::CheckedRegistryAccessor& accessor, const pg::UUID& clipId, const pg::UUID& voiceId, pg::EAudioCategory category, bool loop)
	{
		pg::PlayAudioRequestOneFrameComponent play;
		play.m_ClipID = clipId;
		play.m_VoiceID = voiceId;
		play.m_Category = category;
		play.m_Loop = loop;
		play.m_Volume = 1.0f;
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceOneframe<pg::PlayAudioRequestOneFrameComponent>(ent, std::move(play));
	}
}

pg::SystemAccessDecl sbx::AudioDemoSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::InputStateSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::AudioDemoStateSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::AudioDemoStateSingletonComponent)),
		std::type_index(typeid(pg::PlayAudioRequestOneFrameComponent)),
		std::type_index(typeid(pg::StopAudioRequestOneFrameComponent)),
		std::type_index(typeid(pg::PauseAudioRequestOneFrameComponent)),
		std::type_index(typeid(pg::ResumeAudioRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::AudioDemoSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	if (configView.empty())
	{
		return;
	}
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	auto stateView = accessor.View<sbx::AudioDemoStateSingletonComponent>();
	if (stateView.empty())
	{
		sbx::AudioDemoStateSingletonComponent state;
		state.m_MusicVoiceID = pg::UUID::Generate();
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<sbx::AudioDemoStateSingletonComponent>(ent, std::move(state));
		return;
	}

	auto inputView = accessor.View<const pg::InputStateSingletonComponent>();
	if (inputView.empty())
	{
		return;
	}
	const pg::InputStateSingletonComponent& input = inputView.get<const pg::InputStateSingletonComponent>(inputView.front());
	sbx::AudioDemoStateSingletonComponent& state = stateView.get<sbx::AudioDemoStateSingletonComponent>(stateView.front());

	// S: fire a one-shot sound effect as a fresh overlapping voice.
	if (WasKeyJustPressed(input, pg::PG_KEY_S))
	{
		EmitPlay(accessor, config.m_SampleSoundID, pg::UUID::Generate(), pg::EAudioCategory::eSound, false);
	}

	// M: toggle the looping music on/off.
	if (WasKeyJustPressed(input, pg::PG_KEY_M))
	{
		if (!state.m_MusicPlaying)
		{
			EmitPlay(accessor, config.m_SampleMusicID, state.m_MusicVoiceID, pg::EAudioCategory::eMusic, true);
			state.m_MusicPlaying = true;
			state.m_MusicPaused = false;
		}
		else
		{
			pg::StopAudioRequestOneFrameComponent stop;
			stop.m_VoiceID = state.m_MusicVoiceID;
			pg::ecs::Entity ent = accessor.Create();
			accessor.EmplaceOneframe<pg::StopAudioRequestOneFrameComponent>(ent, std::move(stop));
			state.m_MusicPlaying = false;
			state.m_MusicPaused = false;
		}
	}

	// P: pause or resume the music while it is playing.
	if (WasKeyJustPressed(input, pg::PG_KEY_P) && state.m_MusicPlaying)
	{
		pg::ecs::Entity ent = accessor.Create();
		if (!state.m_MusicPaused)
		{
			pg::PauseAudioRequestOneFrameComponent pause;
			pause.m_VoiceID = state.m_MusicVoiceID;
			accessor.EmplaceOneframe<pg::PauseAudioRequestOneFrameComponent>(ent, std::move(pause));
			state.m_MusicPaused = true;
		}
		else
		{
			pg::ResumeAudioRequestOneFrameComponent resume;
			resume.m_VoiceID = state.m_MusicVoiceID;
			accessor.EmplaceOneframe<pg::ResumeAudioRequestOneFrameComponent>(ent, std::move(resume));
			state.m_MusicPaused = false;
		}
	}
}
