#include "pch.h"
#include "Pigeon/Diffusion/DiffusionSystem.h"

#include <algorithm>
#include <cmath>
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

	// Bilinear resize of an RGB image to (width, height). img2img requires the init image to match the
	// generation size; returns the source unchanged when it already matches.
	pg::Image ResizeImage(const pg::Image& src, uint32_t width, uint32_t height)
	{
		if (src.m_Width == 0 || src.m_Height == 0)
		{
			return pg::Image{};
		}
		if (src.m_Width == width && src.m_Height == height)
		{
			return src;
		}

		pg::Image dst;
		dst.m_Width = width;
		dst.m_Height = height;
		dst.m_Pixels.resize(static_cast<size_t>(width) * height * 3);
		const float scaleX = static_cast<float>(src.m_Width) / width;
		const float scaleY = static_cast<float>(src.m_Height) / height;
		for (uint32_t y = 0; y < height; ++y)
		{
			const float sy = (y + 0.5f) * scaleY - 0.5f;
			int y0 = static_cast<int>(std::floor(sy));
			const float fy = sy - y0;
			int y1 = y0 + 1;
			y0 = std::clamp(y0, 0, static_cast<int>(src.m_Height) - 1);
			y1 = std::clamp(y1, 0, static_cast<int>(src.m_Height) - 1);
			for (uint32_t x = 0; x < width; ++x)
			{
				const float sx = (x + 0.5f) * scaleX - 0.5f;
				int x0 = static_cast<int>(std::floor(sx));
				const float fx = sx - x0;
				int x1 = x0 + 1;
				x0 = std::clamp(x0, 0, static_cast<int>(src.m_Width) - 1);
				x1 = std::clamp(x1, 0, static_cast<int>(src.m_Width) - 1);
				for (int c = 0; c < 3; ++c)
				{
					const float p00 = src.m_Pixels[(static_cast<size_t>(y0) * src.m_Width + x0) * 3 + c];
					const float p10 = src.m_Pixels[(static_cast<size_t>(y0) * src.m_Width + x1) * 3 + c];
					const float p01 = src.m_Pixels[(static_cast<size_t>(y1) * src.m_Width + x0) * 3 + c];
					const float p11 = src.m_Pixels[(static_cast<size_t>(y1) * src.m_Width + x1) * 3 + c];
					const float top = p00 + (p10 - p00) * fx;
					const float bottom = p01 + (p11 - p01) * fx;
					dst.m_Pixels[(static_cast<size_t>(y) * width + x) * 3 + c] = static_cast<uint8_t>(top + (bottom - top) * fy + 0.5f);
				}
			}
		}
		return dst;
	}

	// Estimates the chroma-key colour from the image's four corner patches. For an isolated subject on a
	// flat background the corners are background, so this matches whatever colour the model produced
	// instead of guessing a fixed green.
	glm::ivec3 EstimateKeyFromCorners(const pg::Image& image)
	{
		if (image.m_Pixels.empty())
		{
			return glm::ivec3(0);
		}
		const int width = static_cast<int>(image.m_Width);
		const int height = static_cast<int>(image.m_Height);
		const int patch = std::max(1, std::min(width, height) / 32);
		const int corners[4][2] = { { 0, 0 }, { width - patch, 0 }, { 0, height - patch }, { width - patch, height - patch } };

		long long sum[3] = { 0, 0, 0 };
		long long count = 0;
		for (const int (&corner)[2] : corners)
		{
			for (int dy = 0; dy < patch; ++dy)
			{
				for (int dx = 0; dx < patch; ++dx)
				{
					const int x = std::clamp(corner[0] + dx, 0, width - 1);
					const int y = std::clamp(corner[1] + dy, 0, height - 1);
					const size_t index = (static_cast<size_t>(y) * width + x) * 3;
					sum[0] += image.m_Pixels[index];
					sum[1] += image.m_Pixels[index + 1];
					sum[2] += image.m_Pixels[index + 2];
					++count;
				}
			}
		}
		return glm::ivec3(static_cast<int>(sum[0] / count), static_cast<int>(sum[1] / count), static_cast<int>(sum[2] / count));
	}

	// Composites a foreground over a background by keying out a chroma colour: pixels near keyColor show
	// the background, others keep the foreground, with a soft edge. Used to drop a character generated on
	// a flat key colour onto a real photo without regenerating the background. fg and bg must match size.
	pg::Image ChromaComposite(const pg::Image& fg, const pg::Image& bg, const glm::ivec3& keyColor, float threshold)
	{
		if (fg.m_Pixels.empty() || fg.m_Width != bg.m_Width || fg.m_Height != bg.m_Height)
		{
			return fg;
		}
		pg::Image out;
		out.m_Width = fg.m_Width;
		out.m_Height = fg.m_Height;
		out.m_Pixels.resize(fg.m_Pixels.size());
		const float maxDist = 441.6729f; // sqrt(3) * 255
		const float tNear = std::clamp(threshold * 0.6f, 0.f, 1.f) * maxDist;
		const float tFar = std::clamp(threshold, 0.f, 1.f) * maxDist;
		const float range = std::max(tFar - tNear, 1.f);
		for (size_t i = 0; i + 2 < fg.m_Pixels.size(); i += 3)
		{
			const float dr = static_cast<float>(fg.m_Pixels[i]) - keyColor.r;
			const float dg = static_cast<float>(fg.m_Pixels[i + 1]) - keyColor.g;
			const float db = static_cast<float>(fg.m_Pixels[i + 2]) - keyColor.b;
			const float dist = std::sqrt(dr * dr + dg * dg + db * db);
			const float alpha = std::clamp((dist - tNear) / range, 0.f, 1.f); // 0 = key (background), 1 = foreground
			for (int c = 0; c < 3; ++c)
			{
				const float f = static_cast<float>(fg.m_Pixels[i + c]);
				const float b = static_cast<float>(bg.m_Pixels[i + c]);
				out.m_Pixels[i + c] = static_cast<uint8_t>(f * alpha + b * (1.f - alpha) + 0.5f);
			}
		}
		return out;
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

		if (!request.m_InputImageID.IsNull())
		{
			std::unordered_map<pg::UUID, pg::Image>::const_iterator it = resources.m_InputImageMap.find(request.m_InputImageID);
			if (it != resources.m_InputImageMap.end() && !it->second.m_Pixels.empty())
			{
				params.m_InitImage = ResizeImage(it->second, params.m_Width, params.m_Height);
				params.m_HasInitImage = true;
				params.m_DenoiseStrength = request.m_DenoiseStrength;
			}
			else
			{
				// Requested img2img but the input image is missing/failed to load: warn instead of
				// silently falling back to text2img (which ignores the photo entirely).
				PG_CORE_WARN("DiffusionSystem: img2img input image not found/empty; generating without it");
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

				// Inpaint only the character region into the (preserved) init photo.
				if (request.m_MaskFromSkeleton && params.m_HasInitImage)
				{
					pg::Image mask = pg::RasterizeSkeletonMask(placed, params.m_Width, params.m_Height);
					if (!mask.m_Pixels.empty())
					{
						params.m_Mask = std::move(mask);
						params.m_HasMask = true;
					}
				}
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

			// Optional chroma-key composite: drop the generated subject onto a real background photo,
			// keeping that background pixel-exact (no regeneration). Resolved here so the worker can
			// composite off the main thread.
			bool hasBackground = false;
			pg::Image background;
			const float chromaThreshold = request.m_ChromaKeyThreshold;
			if (!request.m_BackgroundImageID.IsNull())
			{
				std::unordered_map<pg::UUID, pg::Image>::const_iterator backgroundIt = resources.m_InputImageMap.find(request.m_BackgroundImageID);
				if (backgroundIt != resources.m_InputImageMap.end() && !backgroundIt->second.m_Pixels.empty())
				{
					background = ResizeImage(backgroundIt->second, params.m_Width, params.m_Height);
					hasBackground = true;
				}
				else
				{
					PG_CORE_WARN("DiffusionSystem: composite background image not found/empty; skipping composite");
				}
			}

			PG_CORE_INFO("DiffusionSystem: starting generation {0}x{1}, {2} steps, {3} LoRA(s), control={4}, composite={5}", params.m_Width, params.m_Height, params.m_Steps, params.m_Loras.size(), params.m_HasControlHint, hasBackground);

			pg::S_Ptr<pg::DiffusionJob> activeJob = std::make_shared<pg::DiffusionJob>();
			activeJob->m_TargetTextureID = request.m_TargetTextureID;
			activeJob->m_State = pg::EDiffusionJobState::eRunning;

			pg::S_Ptr<pg::DiffusionBackend> backendPtr = backend.m_Backend;
			activeJob->m_Worker = std::thread([activeJob, backendPtr, params, background, chromaThreshold, hasBackground]()
			{
				pg::Image result = backendPtr->Generate(params);
				if (!result.m_Pixels.empty() && hasBackground)
				{
					const glm::ivec3 chromaKey = EstimateKeyFromCorners(result);
					result = ChromaComposite(result, background, chromaKey, chromaThreshold);
				}
				const bool ok = !result.m_Pixels.empty();
				activeJob->m_Result = std::move(result);
				activeJob->m_State = ok ? pg::EDiffusionJobState::eDone : pg::EDiffusionJobState::eFailed;
			});

			job.m_ActiveJob = activeJob;
			break;
		}
	}
}
