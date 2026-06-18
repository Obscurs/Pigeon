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

		// Optional img2img: an input image (resource UUID) the generation starts from, keeping its
		// structure while regenerating. m_DenoiseStrength controls how much is rewritten (0 = unchanged,
		// 1 = ignored / pure text2img). Null ID => pure text2img.
		pg::UUID m_InputImageID;
		float m_DenoiseStrength = 0.75f;

		// Optional single OpenPose ControlNet conditioning: the skeleton resource to rasterize, the
		// strength, and a 2D placement transform applied to its keypoints in the output image's space
		// (identity maps the skeleton's canvas straight onto the output). Null skeleton => no ControlNet.
		pg::UUID m_ControlSkeletonID;
		float m_ControlStrength = 1.f;
		glm::mat3 m_ControlTransform = glm::mat3(1.f);

		// Inpainting: with an Input Image + ControlNet skeleton, confine regeneration to the skeleton's
		// region (a mask built from its bounds) so the rest of the input photo is preserved. Lets a
		// posed character be painted into a faithful background. Ignored without both an input image and
		// a skeleton.
		bool m_MaskFromSkeleton = false;

		// Optional chroma-key composite: generate the subject on a flat key colour (via the prompt),
		// then key that colour out and composite over this background image (resolved like an Input
		// Image). The key colour is auto-detected from the generated image's corners (the background for
		// an isolated subject), so it matches whatever flat colour the model produced. The threshold is
		// the keying tolerance. Keeps the background pixel-exact instead of regenerating it. Null ID =>
		// no composite.
		pg::UUID m_BackgroundImageID;
		float m_ChromaKeyThreshold = 0.35f;

		// Optional skeleton-mask composite: instead of colour-keying, composite the generated subject over
		// m_BackgroundImageID using the OpenPose skeleton's silhouette as a soft alpha mask (white = show
		// the subject, black = show the background). Lets a txt2img + ControlNet figure be placed on a
		// background without img2img (which NaNs here) and without chroma. Takes precedence over the
		// chroma key. Ignored without both a background image and a ControlNet skeleton (m_ControlSkeletonID).
		bool m_CompositeWithSkeletonMask = false;
	};
}
