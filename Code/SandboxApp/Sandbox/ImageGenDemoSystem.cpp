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

	// Step 3: place the posed character over the restyled background. The figure is generated with txt2img
	// + OpenPose ControlNet (poses it) + the SDXL character LoRA (k_DiffusionLoraID, trigger word leading
	// the prompt) on a plain background, then composited onto the restyled background through the figure's
	// per-pixel Alpha Matte (m_CompositeWithMatte) so it integrates by its real outline — replacing the
	// skeleton silhouette, whose geometric bounds left a "gray cloud" halo (ADR 0012; the silhouette is the
	// fallback when the Matting Backend is unavailable). img2img + ControlNet is NOT used: on this SDXL
	// checkpoint it NaNs the latent and decodes to flat grey (min=max=128) regardless of VAE or inpaint mask
	// (ADR 0011). The composite is a CPU blend, so it always renders. (An earlier FLUX-architecture LoRA
	// bound 0 tensors against this SDXL checkpoint — the wrong architecture, not a cause of grey — and was
	// replaced by an Illustrious/SDXL character LoRA.)
	void EmitCompositeRequest(pg::CheckedRegistryAccessor& accessor, const sbx::ImageGenDemoStateSingletonComponent& state)
	{
		pg::GenerateImageRequestOneFrameComponent request;
		request.m_TargetTextureID = sbx::k_CompositeTextureID;
		// "ff7t1f4" is the character LoRA's trigger word — it must lead the prompt for the LoRA to express
		// the trained character. A plain background keeps the silhouette composite clean.
		request.m_Prompt = "ff7t1f4, 1girl, full body, standing, plain white background, simple background, detailed, masterpiece, best quality";
		// Negatives include glow/aura/outline terms: on a plain white background the model tends to paint a
		// soft halo around the subject, which the matte then keeps as foreground and reads as a bright ring.
		request.m_NegativePrompt = "blurry, lowres, deformed, bad anatomy, multiple people, complex background, glowing, glow, aura, halo, rim lighting, backlight, bloom, vignette, white outline, sticker, border";
		request.m_Loras.push_back(pg::GenerateImageLoraRef{ sbx::k_DiffusionLoraID, state.m_CompositeLoraWeight });
		// The OpenPose ControlNet poses the figure and its silhouette masks the composite over the restyled
		// background. Both are gated by the same panel toggle so the figure generation can be tested without
		// the ControlNet (txt2img + LoRA only, shown full-frame) to isolate the flat-grey NaN.
		if (state.m_CompositeUseControlNet)
		{
			request.m_ControlSkeletonID = sbx::k_DiffusionSkeletonID;
			request.m_ControlStrength = 1.0f;
			request.m_BackgroundImageID = sbx::k_BackgroundTextureID;
			// Cut the figure out by its real outline with the matting model and composite it over the
			// restyled background (ADR 0012), eliminating the skeleton silhouette's "gray cloud" halo. The
			// skeleton mask (its tightness knob) remains the fallback when the Matting Backend is unavailable.
			request.m_CompositeWithMatte = true;
			request.m_CompositeMaskScale = state.m_CompositeMaskScale;
			request.m_MatteErodePixels = state.m_CompositeMatteErodePixels;
		}
		accessor.EmplaceOneframe<pg::GenerateImageRequestOneFrameComponent>(accessor.Create(), std::move(request));
	}

	// Optional step 4: an integration pass over the finished composite. The composite (background + matted
	// figure) is fed back as an img2img init and the whole frame is lightly regenerated, harmonising the
	// figure's lighting/texture into the background so it reads as one coherent image rather than a cutout.
	// Pure img2img - no ControlNet (which NaNs the latent to flat grey on this checkpoint, ADR 0011) and no
	// LoRA; the low denoise (m_IntegrationStrength) keeps the composition. The style prompt leads so the
	// harmonisation matches the restyled background's aesthetic.
	void EmitIntegrationRequest(pg::CheckedRegistryAccessor& accessor, const sbx::ImageGenDemoStateSingletonComponent& state)
	{
		pg::GenerateImageRequestOneFrameComponent request;
		request.m_TargetTextureID = sbx::k_IntegratedTextureID;
		request.m_Prompt = std::string(state.m_BackgroundPrompt) + ", 1girl in the scene, cohesive lighting, seamless, integrated, masterpiece, best quality";
		request.m_NegativePrompt = "blurry, lowres, deformed, text, watermark, collage, cutout, sticker, pasted, hard edges";
		request.m_InputImageID = sbx::k_CompositeTextureID;
		request.m_DenoiseStrength = state.m_IntegrationStrength;
		accessor.EmplaceOneframe<pg::GenerateImageRequestOneFrameComponent>(accessor.Create(), std::move(request));
	}

	// Advances the running pipeline: the generation steps wait for their Diffusion Job to finish (seen
	// running, then idle); the instant hint step gates the composite on the restyled background becoming
	// available as an input image (the engine feed-forward), so its img2img init resolves. The optional
	// integration pass gates the same way on the composite becoming an input image.
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
				EmitCompositeRequest(accessor, state);
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
				state.m_Step = state.m_IntegrationPass ? sbx::EImageGenStep::eIntegrate : sbx::EImageGenStep::eDone;
				state.m_SawJobRunning = false;
			}
			break;
		case sbx::EImageGenStep::eIntegrate:
			if (resources.m_InputImageMap.count(sbx::k_CompositeTextureID) > 0)
			{
				EmitIntegrationRequest(accessor, state);
				state.m_Step = sbx::EImageGenStep::eIntegrating;
				state.m_SawJobRunning = false;
			}
			break;
		case sbx::EImageGenStep::eIntegrating:
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
		case sbx::EImageGenStep::eBackground:  return "1/3 restyling background...";
		case sbx::EImageGenStep::eHint:        return "2/3 building pose hint...";
		case sbx::EImageGenStep::eComposite:   return "3/3 compositing figure...";
		case sbx::EImageGenStep::eIntegrate:   return "integration pass...";
		case sbx::EImageGenStep::eIntegrating: return "integration pass...";
		case sbx::EImageGenStep::eDone:        return "done";
		default:                               return "idle";
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
	ImGui::SliderFloat("Composite LoRA weight", &state.m_CompositeLoraWeight, 0.0f, 1.5f);
	ImGui::Checkbox("Composite ControlNet", &state.m_CompositeUseControlNet);
	ImGui::SliderFloat("Composite mask tightness", &state.m_CompositeMaskScale, 0.2f, 1.2f);
	ImGui::SliderInt("Matte edge trim (px)", &state.m_CompositeMatteErodePixels, 0, 30);
	ImGui::Separator();
	// Optional 4th pass: regenerate the composite (img2img) so the figure integrates into the background.
	ImGui::Checkbox("Integration pass", &state.m_IntegrationPass);
	ImGui::SliderFloat("Integration strength", &state.m_IntegrationStrength, 0.1f, 0.8f);

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
	ImGui::TextWrapped("Background -> Pose -> Composite (-> Integration), shown left to right in the scene.");
	ImGui::End();
}
