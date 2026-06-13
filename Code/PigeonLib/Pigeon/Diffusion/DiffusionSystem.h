#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	// Drives text-to-image generation. Lazily creates and loads the resident diffusion backend, then
	// for each GenerateImageRequest assembles the job parameters (resolving Generation Config defaults,
	// LoRA paths, and the OpenPose Control Hint from the resource map) and runs one generation at a time
	// on a background worker thread so the frame loop never blocks. When a job finishes it emits a
	// RegisterGeneratedTextureRequest carrying the result for ResourceManagerSystem to register.
	class DiffusionSystem : public pg::System
	{
	public:
		DiffusionSystem() = default;
		~DiffusionSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
