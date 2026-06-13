#include "pch.h"
#include "Pigeon/Diffusion/DiffusionSystem.h"

#include <thread>
#include <utility>

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionBackend.h"
#include "Pigeon/Diffusion/DiffusionBackendSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionJob.h"
#include "Pigeon/Diffusion/DiffusionJobSingletonComponent.h"
#include "Pigeon/Diffusion/GenerateImageRequestOneFrameComponent.h"
#include "Pigeon/Diffusion/OpenPoseHint.h"
#include "Pigeon/Diffusion/RegisterGeneratedTextureRequestOneFrameComponent.h"
#include "Pigeon/ECS/World.h"

namespace
{
	// Picks the single resident checkpoint/ControlNet path (the first declared of each). One checkpoint
	// per session, so the first entry is authoritative.
	std::string FirstPath(const std::unordered_map<pg::UUID, std::string>& map)
	{
		if (map.empty())
		{
			return std::string();
		}
		return map.begin()->second;
	}

	// Resolves a request + engine defaults + resource paths into the backend's job parameters.
	pg::DiffusionJobParams AssembleParams(const pg::GenerateImageRequestOneFrameComponent& request, const pg::EngineConfigSingletonComponent& config, const pg::ResourceMapSingletonComponent& resources)
	{
		pg::DiffusionJobParams params;
		params.m_Prompt = request.m_Prompt;
		params.m_NegativePrompt = request.m_NegativePrompt;
		params.m_Seed = request.m_Seed;
		params.m_Steps = request.m_Steps > 0 ? request.m_Steps : config.m_DiffusionSteps;
		params.m_CfgScale = request.m_CfgScale > 0.f ? request.m_CfgScale : config.m_DiffusionCfgScale;
		params.m_Sampler = !request.m_Sampler.empty() ? request.m_Sampler : config.m_DiffusionSampler;
		params.m_Width = request.m_Width > 0 ? request.m_Width : config.m_DiffusionWidth;
		params.m_Height = request.m_Height > 0 ? request.m_Height : config.m_DiffusionHeight;
		params.m_ClipSkip = request.m_ClipSkip > 0 ? request.m_ClipSkip : config.m_DiffusionClipSkip;

		for (const pg::GenerateImageLoraRef& loraRef : request.m_Loras)
		{
			std::unordered_map<pg::UUID, std::string>::const_iterator it = resources.m_LoraMap.find(loraRef.m_LoraID);
			if (it != resources.m_LoraMap.end())
			{
				params.m_Loras.push_back(pg::DiffusionLora{ it->second, loraRef.m_Weight });
			}
		}

		if (!request.m_ControlSkeletonID.IsNull())
		{
			std::unordered_map<pg::UUID, pg::S_Ptr<pg::OpenPoseSkeleton>>::const_iterator it = resources.m_OpenPoseSkeletonMap.find(request.m_ControlSkeletonID);
			if (it != resources.m_OpenPoseSkeletonMap.end() && it->second != nullptr)
			{
				const pg::OpenPoseSkeleton& skeleton = *it->second;
				const glm::mat3 transform = request.m_ControlTransform * pg::MakeCanvasToImageTransform(skeleton.GetCanvasWidth(), skeleton.GetCanvasHeight(), params.m_Width, params.m_Height);
				const std::array<pg::OpenPoseKeypoint, pg::OpenPoseSkeleton::k_JointCount> placed = pg::TransformKeypoints(skeleton.GetKeypoints(), transform);
				params.m_ControlHint = pg::RasterizeOpenPoseHint(placed, params.m_Width, params.m_Height);
				params.m_HasControlHint = true;
				params.m_ControlStrength = request.m_ControlStrength;
			}
		}

		return params;
	}
}

pg::SystemAccessDecl pg::DiffusionSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::GenerateImageRequestOneFrameComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::DiffusionBackendSingletonComponent)),
		std::type_index(typeid(pg::DiffusionJobSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::DiffusionBackendSingletonComponent)),
		std::type_index(typeid(pg::DiffusionJobSingletonComponent)),
		std::type_index(typeid(pg::RegisterGeneratedTextureRequestOneFrameComponent)),
	};
	return decl;
}

void pg::DiffusionSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
	auto resourceView = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (configView.empty() || resourceView.empty())
	{
		return;
	}

	// Lazily create the backend + job singletons (deferred -> visible next frame), like the other
	// startup singletons.
	auto backendView = accessor.View<pg::DiffusionBackendSingletonComponent>();
	auto jobView = accessor.View<pg::DiffusionJobSingletonComponent>();
	if (backendView.empty() || jobView.empty())
	{
		if (backendView.empty())
		{
			pg::DiffusionBackendSingletonComponent backendComponent;
			backendComponent.m_Backend = pg::DiffusionBackend::Create();
			accessor.EmplaceDeferred<pg::DiffusionBackendSingletonComponent>(accessor.Create(), std::move(backendComponent));
		}
		if (jobView.empty())
		{
			accessor.EmplaceDeferred<pg::DiffusionJobSingletonComponent>(accessor.Create(), pg::DiffusionJobSingletonComponent{});
		}
		return;
	}

	const pg::EngineConfigSingletonComponent& config = configView.get<const pg::EngineConfigSingletonComponent>(configView.front());
	const pg::ResourceMapSingletonComponent& resources = resourceView.get<const pg::ResourceMapSingletonComponent>(resourceView.front());
	pg::DiffusionBackendSingletonComponent& backend = backendView.get<pg::DiffusionBackendSingletonComponent>(backendView.front());
	pg::DiffusionJobSingletonComponent& job = jobView.get<pg::DiffusionJobSingletonComponent>(jobView.front());

	// Load the resident checkpoint (+ ControlNet) exactly once.
	if (!backend.m_LoadAttempted)
	{
		backend.m_LoadAttempted = true;
		const std::string checkpointPath = FirstPath(resources.m_CheckpointMap);
		if (backend.m_Backend != nullptr && !checkpointPath.empty())
		{
			PG_CORE_INFO("DiffusionSystem: loading checkpoint '{0}' (+{1} ControlNet(s), {2} VAE(s))", checkpointPath, resources.m_ControlNetMap.size(), resources.m_VaeMap.size());
			backend.m_Backend->LoadCheckpoint(checkpointPath, FirstPath(resources.m_ControlNetMap), FirstPath(resources.m_VaeMap));
		}
		else
		{
			PG_CORE_WARN("DiffusionSystem: no checkpoint in the resource map ({0} declared); text-to-image disabled", resources.m_CheckpointMap.size());
		}
	}

	// Reap a finished job: publish the result for registration, then clear the slot.
	if (job.m_ActiveJob != nullptr)
	{
		const pg::EDiffusionJobState state = job.m_ActiveJob->m_State.load();
		if (state == pg::EDiffusionJobState::eDone)
		{
			PG_CORE_INFO("DiffusionSystem: generation complete ({0}x{1})", job.m_ActiveJob->m_Result.m_Width, job.m_ActiveJob->m_Result.m_Height);
			pg::RegisterGeneratedTextureRequestOneFrameComponent registration;
			registration.m_TextureID = job.m_ActiveJob->m_TargetTextureID;
			registration.m_Image = job.m_ActiveJob->m_Result;
			accessor.EmplaceOneframe<pg::RegisterGeneratedTextureRequestOneFrameComponent>(accessor.Create(), std::move(registration));
			job.m_ActiveJob.reset();
		}
		else if (state == pg::EDiffusionJobState::eFailed)
		{
			PG_CORE_WARN("DiffusionSystem: generation failed (backend returned an empty image)");
			job.m_ActiveJob.reset();
		}
	}

	// Launch one generation when idle.
	if (job.m_ActiveJob == nullptr)
	{
		auto requestView = accessor.View<const pg::GenerateImageRequestOneFrameComponent>();
		for (pg::ecs::Entity requestEntity : requestView)
		{
			if (backend.m_Backend == nullptr || !backend.m_Backend->IsLoaded())
			{
				// A request arrived but no checkpoint is resident (load failed, or no checkpoint was
				// declared in a manifest). The result UUID stays unregistered and shows the default texture.
				PG_CORE_WARN("DiffusionSystem: GenerateImageRequest received but no checkpoint is loaded; ignoring");
				break;
			}

			const pg::GenerateImageRequestOneFrameComponent& request = requestView.get<const pg::GenerateImageRequestOneFrameComponent>(requestEntity);
			const pg::DiffusionJobParams params = AssembleParams(request, config, resources);
			PG_CORE_INFO("DiffusionSystem: starting generation {0}x{1}, {2} steps, {3} LoRA(s), control={4}", params.m_Width, params.m_Height, params.m_Steps, params.m_Loras.size(), params.m_HasControlHint);

			pg::S_Ptr<pg::DiffusionJob> activeJob = std::make_shared<pg::DiffusionJob>();
			activeJob->m_TargetTextureID = request.m_TargetTextureID;
			activeJob->m_State = pg::EDiffusionJobState::eRunning;

			pg::S_Ptr<pg::DiffusionBackend> backendPtr = backend.m_Backend;
			activeJob->m_Worker = std::thread([activeJob, backendPtr, params]()
			{
				pg::Image result = backendPtr->Generate(params);
				const bool ok = !result.m_Pixels.empty();
				activeJob->m_Result = std::move(result);
				activeJob->m_State = ok ? pg::EDiffusionJobState::eDone : pg::EDiffusionJobState::eFailed;
			});

			job.m_ActiveJob = activeJob;
			break;
		}
	}
}
