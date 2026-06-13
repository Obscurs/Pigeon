#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Pigeon/Core/UUID.h"

namespace pg
{
	// A LoRA selected for one generation: the LoRA resource to apply and its weight.
	struct GenerateImageLoraRef
	{
		pg::UUID m_LoraID;
		float m_Weight = 1.f;
	};

	// App-emitted request (engine-typed, so the engine DiffusionSystem can read it) to generate one
	// image. The result is registered into the resource map's texture map under m_TargetTextureID
	// (caller-assigned, like an audio Voice Handle). Generation Config fields left at their zero/empty
	// sentinel fall back to the engine defaults; prompt/negative/seed are always honoured.
	struct GenerateImageRequestOneFrameComponent
	{
		pg::UUID m_TargetTextureID;
		std::string m_Prompt;
		std::string m_NegativePrompt;
		int64_t m_Seed = -1;

		int m_Steps = 0;
		float m_CfgScale = 0.f;
		std::string m_Sampler;
		unsigned int m_Width = 0;
		unsigned int m_Height = 0;
		int m_ClipSkip = 0;

		std::vector<GenerateImageLoraRef> m_Loras;

		// Optional single OpenPose ControlNet conditioning: the skeleton resource to rasterize, the
		// strength, and a 2D placement transform applied to its keypoints in the output image's space
		// (identity maps the skeleton's canvas straight onto the output). Null skeleton => no ControlNet.
		pg::UUID m_ControlSkeletonID;
		float m_ControlStrength = 1.f;
		glm::mat3 m_ControlTransform = glm::mat3(1.f);
	};
}
