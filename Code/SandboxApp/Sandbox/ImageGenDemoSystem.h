#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	// Text-to-image demo: on each G keypress it emits a GenerateImageRequest for the resident checkpoint
	// with a LoRA and an OpenPose ControlNet hint (the loaded skeleton), targeting the generated-texture
	// UUID that SceneSetupSystem's display sprite samples. Edge-triggered so the user can retry after the
	// checkpoint loads; DiffusionSystem serialises and drops overlapping/not-ready requests.
	class ImageGenDemoSystem : public pg::System
	{
	public:
		ImageGenDemoSystem() = default;
		~ImageGenDemoSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
