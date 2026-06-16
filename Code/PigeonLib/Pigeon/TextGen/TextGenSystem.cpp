#include "pch.h"
#include "Pigeon/TextGen/TextGenSystem.h"

#include <thread>
#include <utility>

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/TextGen/GenerateTextRequestOneFrameComponent.h"
#include "Pigeon/TextGen/TextGenBackend.h"
#include "Pigeon/TextGen/TextGenBackendSingletonComponent.h"
#include "Pigeon/TextGen/TextGenJob.h"
#include "Pigeon/TextGen/TextGenJobSingletonComponent.h"
#include "Pigeon/TextGen/TextGenResultSingletonComponent.h"

namespace
{
	// Picks the single resident model path (the first declared). One model per session, so the first
	// entry is authoritative.
	std::string FirstPath(const std::unordered_map<pg::UUID, std::string>& map)
	{
		if (map.empty())
		{
			return std::string();
		}
		return map.begin()->second;
	}

	// Resolves a request + engine defaults into the backend's job parameters. Unset (<= 0) config fields
	// fall back to the engine Text Generation Config defaults; prompt/system prompt/seed are always taken
	// from the request.
	pg::TextGenParams AssembleParams(const pg::GenerateTextRequestOneFrameComponent& request, const pg::EngineConfigSingletonComponent& config)
	{
		pg::TextGenParams params;
		params.m_Prompt = request.m_Prompt;
		params.m_SystemPrompt = request.m_SystemPrompt;
		params.m_Seed = request.m_Seed;
		params.m_MaxTokens = request.m_MaxTokens > 0 ? request.m_MaxTokens : config.m_TextGenMaxTokens;
		params.m_Temperature = request.m_Temperature > 0.f ? request.m_Temperature : config.m_TextGenTemperature;
		params.m_TopP = request.m_TopP > 0.f ? request.m_TopP : config.m_TextGenTopP;
		return params;
	}
}

pg::SystemAccessDecl pg::TextGenSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::GenerateTextRequestOneFrameComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::TextGenBackendSingletonComponent)),
		std::type_index(typeid(pg::TextGenJobSingletonComponent)),
		std::type_index(typeid(pg::TextGenResultSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::TextGenBackendSingletonComponent)),
		std::type_index(typeid(pg::TextGenJobSingletonComponent)),
		std::type_index(typeid(pg::TextGenResultSingletonComponent)),
	};
	return decl;
}

void pg::TextGenSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
	auto resourceView = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (configView.empty() || resourceView.empty())
	{
		return;
	}

	// Lazily create the backend + job + result singletons (deferred -> visible next frame), like the
	// other startup singletons.
	auto backendView = accessor.View<pg::TextGenBackendSingletonComponent>();
	auto jobView = accessor.View<pg::TextGenJobSingletonComponent>();
	auto resultView = accessor.View<pg::TextGenResultSingletonComponent>();
	if (backendView.empty() || jobView.empty() || resultView.empty())
	{
		if (backendView.empty())
		{
			pg::TextGenBackendSingletonComponent backendComponent;
			backendComponent.m_Backend = pg::TextGenBackend::Create();
			accessor.EmplaceDeferred<pg::TextGenBackendSingletonComponent>(accessor.Create(), std::move(backendComponent));
		}
		if (jobView.empty())
		{
			accessor.EmplaceDeferred<pg::TextGenJobSingletonComponent>(accessor.Create(), pg::TextGenJobSingletonComponent{});
		}
		if (resultView.empty())
		{
			accessor.EmplaceDeferred<pg::TextGenResultSingletonComponent>(accessor.Create(), pg::TextGenResultSingletonComponent{});
		}
		return;
	}

	const pg::EngineConfigSingletonComponent& config = configView.get<const pg::EngineConfigSingletonComponent>(configView.front());
	const pg::ResourceMapSingletonComponent& resources = resourceView.get<const pg::ResourceMapSingletonComponent>(resourceView.front());
	pg::TextGenBackendSingletonComponent& backend = backendView.get<pg::TextGenBackendSingletonComponent>(backendView.front());
	pg::TextGenJobSingletonComponent& job = jobView.get<pg::TextGenJobSingletonComponent>(jobView.front());
	pg::TextGenResultSingletonComponent& results = resultView.get<pg::TextGenResultSingletonComponent>(resultView.front());

	// Load the resident model exactly once.
	if (!backend.m_LoadAttempted)
	{
		backend.m_LoadAttempted = true;
		const std::string modelPath = FirstPath(resources.m_LanguageModelMap);
		if (backend.m_Backend != nullptr && !modelPath.empty())
		{
			PG_CORE_INFO("TextGenSystem: loading language model '{0}' ({1} GPU layers)", modelPath, config.m_TextGenGpuLayers);
			backend.m_Backend->LoadModel(modelPath, config.m_TextGenGpuLayers);
		}
		else
		{
			PG_CORE_WARN("TextGenSystem: no language model in the resource map ({0} declared); text generation disabled", resources.m_LanguageModelMap.size());
		}
	}

	// Reap a finished job: publish the result into the result store, then clear the slot.
	if (job.m_ActiveJob != nullptr)
	{
		const pg::ETextGenJobState state = job.m_ActiveJob->m_State.load();
		if (state == pg::ETextGenJobState::eDone)
		{
			PG_CORE_INFO("TextGenSystem: generation complete ({0} chars)", job.m_ActiveJob->m_Result.size());
			results.m_Results[job.m_ActiveJob->m_TargetTextID] = job.m_ActiveJob->m_Result;
			job.m_ActiveJob.reset();
		}
		else if (state == pg::ETextGenJobState::eFailed)
		{
			PG_CORE_WARN("TextGenSystem: generation failed (backend returned an empty string)");
			job.m_ActiveJob.reset();
		}
	}

	// Launch one generation when idle.
	if (job.m_ActiveJob == nullptr)
	{
		auto requestView = accessor.View<const pg::GenerateTextRequestOneFrameComponent>();
		for (pg::ecs::Entity requestEntity : requestView)
		{
			if (backend.m_Backend == nullptr || !backend.m_Backend->IsLoaded())
			{
				// A request arrived but no model is resident (load failed, or none was declared in a
				// manifest). The target UUID stays unset in the result store.
				PG_CORE_WARN("TextGenSystem: GenerateTextRequest received but no model is loaded; ignoring");
				break;
			}

			const pg::GenerateTextRequestOneFrameComponent& request = requestView.get<const pg::GenerateTextRequestOneFrameComponent>(requestEntity);
			const pg::TextGenParams params = AssembleParams(request, config);

			PG_CORE_INFO("TextGenSystem: starting generation (max {0} tokens, temp {1})", params.m_MaxTokens, params.m_Temperature);

			pg::S_Ptr<pg::TextGenJob> activeJob = std::make_shared<pg::TextGenJob>();
			activeJob->m_TargetTextID = request.m_TargetTextID;
			activeJob->m_State = pg::ETextGenJobState::eRunning;

			pg::S_Ptr<pg::TextGenBackend> backendPtr = backend.m_Backend;
			activeJob->m_Worker = std::thread([activeJob, backendPtr, params]()
			{
				std::string result = backendPtr->Generate(params);
				const bool ok = !result.empty();
				activeJob->m_Result = std::move(result);
				activeJob->m_State = ok ? pg::ETextGenJobState::eDone : pg::ETextGenJobState::eFailed;
			});

			job.m_ActiveJob = activeJob;
			break;
		}
	}
}
