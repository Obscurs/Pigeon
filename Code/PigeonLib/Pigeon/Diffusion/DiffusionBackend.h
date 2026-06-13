#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Pigeon/Core/Core.h"
#include "Pigeon/Diffusion/Image.h"

namespace pg
{
	// One LoRA applied to a single generation: the resolved file path of the LoRA weights and the
	// strength it is applied at. Selected per Generate Image Request on top of the resident checkpoint.
	struct DiffusionLora
	{
		std::string m_Path;
		float m_Weight = 1.f;
	};

	// The fully-resolved inputs of one generation, assembled by DiffusionSystem from a Generate Image
	// Request, the engine defaults, and the resident/resolved model paths. Everything the backend needs
	// to run a single denoising loop, with no ECS or resource-map knowledge.
	struct DiffusionJobParams
	{
		std::string m_Prompt;
		std::string m_NegativePrompt;
		int m_Steps = 20;
		float m_CfgScale = 7.f;
		std::string m_Sampler = "euler_a";
		uint32_t m_Width = 512;
		uint32_t m_Height = 512;
		int m_ClipSkip = 1;
		int64_t m_Seed = -1;

		std::vector<DiffusionLora> m_Loras;

		// Optional img2img init image (the "blend an input image" path) and how strongly it is replaced.
		bool m_HasInitImage = false;
		pg::Image m_InitImage;
		float m_DenoiseStrength = 0.75f;

		// Optional single ControlNet hint (e.g. the rasterized OpenPose skeleton) and its strength.
		bool m_HasControlHint = false;
		pg::Image m_ControlHint;
		float m_ControlStrength = 1.f;
	};

	// Platform-abstracted text-to-image inference runtime. The concrete backend (stable-diffusion.cpp
	// for real builds, a deterministic no-op for tests) is selected by Create() using the same
	// renderer-API switch as the other platform resources. Held in DiffusionBackendSingletonComponent.
	class DiffusionBackend
	{
	public:
		virtual ~DiffusionBackend() = default;

		// Loads the resident checkpoint and, when non-empty, a single ControlNet model and an external
		// VAE (overriding the checkpoint's baked VAE — needed for good SDXL output). Called once at
		// startup; returns true on success. The loaded model stays resident for the session.
		virtual bool LoadCheckpoint(const std::string& checkpointPath, const std::string& controlNetPath, const std::string& vaePath) = 0;
		virtual bool IsLoaded() const = 0;

		// Runs one generation synchronously (invoked on a background worker thread). Returns an empty
		// image (zero size) on failure or when no checkpoint is loaded.
		virtual pg::Image Generate(const DiffusionJobParams& params) = 0;

		static pg::S_Ptr<DiffusionBackend> Create();
	};
}
