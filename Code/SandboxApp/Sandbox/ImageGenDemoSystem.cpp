#include "Sandbox/ImageGenDemoSystem.h"

#include <string>
#include <utility>

#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionBackend.h"
#include "Pigeon/Diffusion/DiffusionBackendSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionJobSingletonComponent.h"
#include "Pigeon/Diffusion/GenerateImageRequestOneFrameComponent.h"
#include "Pigeon/Diffusion/RasterizeOpenPoseHintRequestOneFrameComponent.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/ImageGenDemoIds.h"
#include "Sandbox/ImageGenDemoStateSingletonComponent.h"

#include <imgui.h>

namespace
{

	// Step 3 img2img denoise, applied only inside the skeleton inpaint mask (the figure region). It must
	// be high so the ControlNet has the regeneration freedom to impose the pose there; the mask keeps that
	// high denoise from touching the rest of the restyled room, which stays pixel-exact. Below 1.0 so
	// sd.cpp still encodes the init image as the starting point.
	const float k_CompositeDenoiseStrength = 0.9f;

	// Maps the panel's consistency slider (1 = most faithful to the original photo) to an img2img
	// denoise strength for the background restyle: more consistency -> less of the photo is rewritten.
	float ConsistencyToDenoise(float consistency)
	{
		const float clamped = consistency < 0.f ? 0.f : (consistency > 1.f ? 1.f : consistency);
		return 0.2f + (1.f - clamped) * 0.6f; // [0.2, 0.8]
	}

	bool GeneratePressed(pg::CheckedRegistryAccessor& accessor)
	{
		auto keyView = accessor.View<const pg::KeyPressedEventComponent>();
		for (pg::ecs::Entity ent : keyView)
		{
			if (keyView.get<const pg::KeyPressedEventComponent>(ent).m_KeyCode == pg::PG_KEY_G)
			{
				return true;
			}
		}
		return false;
	}

	// Step 1: restyle the original living-room photo (img2img) with the panel's style prompt, keeping its
	// structure. No LoRA, no ControlNet -> the figure is added later, in the composite step.
	void EmitBackgroundRequest(pg::CheckedRegistryAccessor& accessor, const sbx::ImageGenDemoStateSingletonComponent& state)
	{
		pg::GenerateImageRequestOneFrameComponent request;
		request.m_TargetTextureID = sbx::k_BackgroundTextureID;
		request.m_Prompt = std::string(state.m_BackgroundPrompt) + ", masterpiece, best quality";
		request.m_NegativePrompt = "blurry, lowres, deformed, text, watermark, people, person";
		request.m_InputImageID = sbx::k_LivingRoomImageID;
		request.m_DenoiseStrength = ConsistencyToDenoise(state.m_Consistency);
		accessor.EmplaceOneframe<pg::GenerateImageRequestOneFrameComponent>(accessor.Create(), std::move(request));
	}

	// Step 2: rasterize the OpenPose skeleton into a shown texture (synchronous, in the engine).
	void EmitHintRequest(pg::CheckedRegistryAccessor& accessor)
	{
		pg::RasterizeOpenPoseHintRequestOneFrameComponent request;
		request.m_SkeletonID = sbx::k_DiffusionSkeletonID;
		request.m_TargetTextureID = sbx::k_HintTextureID;
		accessor.EmplaceOneframe<pg::RasterizeOpenPoseHintRequestOneFrameComponent>(accessor.Create(), std::move(request));
	}

	// Step 3: paint the figure into the restyled background. img2img on that background (the previous
	// step's result, fed forward as the init image) + OpenPose ControlNet (poses the figure), confined to
	// the skeleton's region by an inpaint mask (m_MaskFromSkeleton) so a high denoise can impose the pose
	// on the figure while the rest of the restyled room stays pixel-exact. This is regeneration-masking
	// (no composite seams), the only way plain img2img gives both a strong pose and the kept background.
	// (The grey that blocked this earlier was the character LoRA, now disabled.)
	void EmitCompositeRequest(pg::CheckedRegistryAccessor& accessor)
	{
		pg::GenerateImageRequestOneFrameComponent request;
		request.m_TargetTextureID = sbx::k_CompositeTextureID;
		request.m_Prompt = "1girl, full body, standing in a room, detailed, masterpiece, best quality";
		request.m_NegativePrompt = "blurry, lowres, deformed, bad anatomy, multiple people";
		// Character LoRA disabled: it binds 0 tensors against this checkpoint ("0/1128 applied") and
		// corrupted the generation to grey. Re-add once it binds correctly.
		request.m_InputImageID = sbx::k_BackgroundTextureID;
		request.m_DenoiseStrength = k_CompositeDenoiseStrength;
		request.m_ControlSkeletonID = sbx::k_DiffusionSkeletonID;
		request.m_ControlStrength = 1.0f;
		request.m_MaskFromSkeleton = true;
		accessor.EmplaceOneframe<pg::GenerateImageRequestOneFrameComponent>(accessor.Create(), std::move(request));
	}

	// Advances the running pipeline: the two generation steps wait for their Diffusion Job to finish
	// (seen running, then idle); the instant hint step gates the composite on the restyled background
	// becoming available as an input image (the engine feed-forward), so its img2img init resolves.
	void AdvancePipeline(pg::CheckedRegistryAccessor& accessor, sbx::ImageGenDemoStateSingletonComponent& state, const pg::ResourceMapSingletonComponent& resources, bool jobRunning)
	{
		switch (state.m_Step)
		{
		case sbx::EImageGenStep::eBackground:
			if (jobRunning)
			{
				state.m_SawJobRunning = true;
			}
			else if (state.m_SawJobRunning)
			{
				EmitHintRequest(accessor);
				state.m_Step = sbx::EImageGenStep::eHint;
				state.m_SawJobRunning = false;
			}
			break;
		case sbx::EImageGenStep::eHint:
			if (resources.m_InputImageMap.count(sbx::k_BackgroundTextureID) > 0)
			{
				EmitCompositeRequest(accessor);
				state.m_Step = sbx::EImageGenStep::eComposite;
				state.m_SawJobRunning = false;
			}
			break;
		case sbx::EImageGenStep::eComposite:
			if (jobRunning)
			{
				state.m_SawJobRunning = true;
			}
			else if (state.m_SawJobRunning)
			{
				state.m_Step = sbx::EImageGenStep::eDone;
				state.m_SawJobRunning = false;
			}
			break;
		default:
			break;
		}
	}

	void StartPipeline(pg::CheckedRegistryAccessor& accessor, sbx::ImageGenDemoStateSingletonComponent& state)
	{
		EmitBackgroundRequest(accessor, state);
		state.m_Step = sbx::EImageGenStep::eBackground;
		state.m_SawJobRunning = false;
	}

	bool IsPipelineIdle(const sbx::ImageGenDemoStateSingletonComponent& state)
	{
		return state.m_Step == sbx::EImageGenStep::eIdle || state.m_Step == sbx::EImageGenStep::eDone;
	}

	const char* StepLabel(sbx::EImageGenStep step)
	{
		switch (step)
		{
		case sbx::EImageGenStep::eBackground: return "1/3 restyling background...";
		case sbx::EImageGenStep::eHint:       return "2/3 building pose hint...";
		case sbx::EImageGenStep::eComposite:  return "3/3 compositing figure...";
		case sbx::EImageGenStep::eDone:       return "done";
		default:                              return "idle";
		}
	}
}

pg::SystemAccessDecl sbx::ImageGenDemoSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::KeyPressedEventComponent)),
		std::type_index(typeid(pg::DiffusionBackendSingletonComponent)),
		std::type_index(typeid(pg::DiffusionJobSingletonComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::ImageGenDemoStateSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::ImageGenDemoStateSingletonComponent)),
		std::type_index(typeid(pg::GenerateImageRequestOneFrameComponent)),
		std::type_index(typeid(pg::RasterizeOpenPoseHintRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::ImageGenDemoSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	// The engine diffusion singletons are created lazily by DiffusionSystem; wait until they exist.
	auto backendView = accessor.View<const pg::DiffusionBackendSingletonComponent>();
	auto jobView = accessor.View<const pg::DiffusionJobSingletonComponent>();
	auto resourceView = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (backendView.empty() || jobView.empty() || resourceView.empty())
	{
		return;
	}

	// Seed the editable panel state once (deferred -> visible next frame); thereafter the user owns it.
	auto stateView = accessor.View<sbx::ImageGenDemoStateSingletonComponent>();
	if (stateView.empty())
	{
		accessor.EmplaceDeferred<sbx::ImageGenDemoStateSingletonComponent>(accessor.Create(), sbx::ImageGenDemoStateSingletonComponent{});
		return;
	}

	sbx::ImageGenDemoStateSingletonComponent& state = stateView.get<sbx::ImageGenDemoStateSingletonComponent>(stateView.front());
	const pg::DiffusionBackendSingletonComponent& backend = backendView.get<const pg::DiffusionBackendSingletonComponent>(backendView.front());
	const pg::DiffusionJobSingletonComponent& job = jobView.get<const pg::DiffusionJobSingletonComponent>(jobView.front());
	const pg::ResourceMapSingletonComponent& resources = resourceView.get<const pg::ResourceMapSingletonComponent>(resourceView.front());

	const bool backendReady = backend.m_Backend != nullptr && backend.m_Backend->IsLoaded();
	const bool jobRunning = job.m_ActiveJob != nullptr;

	// Drive the running pipeline forward (ImGui-independent so it runs in headless tests too).
	AdvancePipeline(accessor, state, resources, jobRunning);

	// The G key is the Generate keyboard shortcut (and the headless-test entry point, since the ImGui
	// button below cannot be pressed without an ImGui context). Both start the same pipeline.
	if (backendReady && IsPipelineIdle(state) && GeneratePressed(accessor))
	{
		StartPipeline(accessor, state);
	}

	// The test build pushes no ImGuiLayer, so no ImGui context exists there; guard every call.
	if (ImGui::GetCurrentContext() == nullptr)
	{
		return;
	}

	ImGui::Begin("Image Generation");
	ImGui::TextWrapped("Background style prompt (restyles the photo, keeps its structure):");
	ImGui::InputTextMultiline("##bgprompt", state.m_BackgroundPrompt, sizeof(state.m_BackgroundPrompt), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 3));
	ImGui::SliderFloat("Consistency", &state.m_Consistency, 0.0f, 1.0f);

	if (backendReady && IsPipelineIdle(state))
	{
		if (ImGui::Button("Generate (G)"))
		{
			StartPipeline(accessor, state);
		}
	}
	else if (!backendReady)
	{
		ImGui::BeginDisabled();
		ImGui::Button("Loading checkpoint...");
		ImGui::EndDisabled();
	}
	else
	{
		ImGui::BeginDisabled();
		ImGui::Button("Generating...");
		ImGui::EndDisabled();
	}

	ImGui::Separator();
	ImGui::TextWrapped("Status: %s", StepLabel(state.m_Step));
	ImGui::TextWrapped("Background -> Pose -> Composite, shown left to right in the scene.");
	ImGui::End();
}
