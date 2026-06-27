#pragma once

#include <string>

#include "Pigeon/Core/Core.h"
#include "Pigeon/Diffusion/Image.h"

namespace pg
{
	// Platform-abstracted image-matting inference runtime. Produces a per-pixel foreground Alpha Matte
	// for an input image (a generated figure), used as the composite alpha in place of the geometric
	// OpenPose skeleton silhouette so the character integrates into the background without a halo
	// (ADR 0012). The concrete backend is selected by Create() using the same renderer-API switch as the
	// other platform resources (OnnxMattingBackend for real builds, a deterministic no-op for tests).
	// Held in MattingBackendSingletonComponent, created and loaded once by DiffusionSystem.
	class MattingBackend
	{
	public:
		virtual ~MattingBackend() = default;

		// Loads the resident matting model once; returns true on success. The model stays resident for the
		// session (one model per session, like a diffusion Checkpoint).
		virtual bool LoadModel(const std::string& modelPath) = 0;
		virtual bool IsLoaded() const = 0;

		// Runs one segmentation synchronously (invoked on the diffusion worker thread). Returns an RGB
		// Alpha Matte the SAME size as input — white = foreground, black = background, channels replicated
		// so it plugs straight into the mask-composite alpha. Returns an empty image (zero size) on failure
		// or when no model is loaded, so the caller can fall back to the skeleton silhouette.
		virtual pg::Image Matte(const pg::Image& input) = 0;

		static pg::S_Ptr<MattingBackend> Create();
	};
}
