#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	// Text-to-image demo driving a 3-step pipeline (ADR 0011): (1) restyle the original photo with an
	// editable style prompt + consistency slider (img2img), (2) rasterize the OpenPose pose hint, (3)
	// paint the character into the restyled background (img2img on it + OpenPose ControlNet + character
	// LoRA, skeleton-masked). One Generate button (G shortcut) runs all three back-to-back, advancing as
	// each Diffusion Job completes; the three result textures are shown side by side by SceneSetupSystem.
	// Owns the ImageGenDemoStateSingletonComponent (panel state + pipeline progress).
	class ImageGenDemoSystem : public pg::System
	{
	public:
		ImageGenDemoSystem() = default;
		~ImageGenDemoSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
