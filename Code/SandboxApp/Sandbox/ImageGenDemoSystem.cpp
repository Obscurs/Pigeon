#include "Sandbox/ImageGenDemoSystem.h"

#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Diffusion/GenerateImageRequestOneFrameComponent.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/ImageGenDemoIds.h"

pg::SystemAccessDecl sbx::ImageGenDemoSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::KeyPressedEventComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::GenerateImageRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::ImageGenDemoSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	// Fire on each distinct G keypress (edge-triggered), so the user can retry after the checkpoint
	// finishes loading. DiffusionSystem ignores the request while a job is running or the backend is
	// not yet loaded, so repeated presses never queue up or overlap generations.
	bool generatePressed = false;
	auto keyView = accessor.View<const pg::KeyPressedEventComponent>();
	for (pg::ecs::Entity ent : keyView)
	{
		if (keyView.get<const pg::KeyPressedEventComponent>(ent).m_KeyCode == pg::PG_KEY_G)
		{
			generatePressed = true;
			break;
		}
	}
	if (!generatePressed)
	{
		return;
	}

	pg::GenerateImageRequestOneFrameComponent request;
	request.m_TargetTextureID = sbx::k_GeneratedTextureID;
	// "Lin" is the LoRA's trigger word; the LoRA itself is applied below by UUID (resolved to its file
	// path and passed to the backend natively), so the prompt only needs to invoke the character.
	request.m_Prompt = "lin, full body, standing, wearing pajamas, isolated on a solid chroma key green background, masterpiece, best quality";
	request.m_NegativePrompt = "blurry, lowres, deformed, bad anatomy, shadow, multiple people";
	request.m_Loras.push_back(pg::GenerateImageLoraRef{ sbx::k_DiffusionLoraID, 1.0f });
	request.m_ControlSkeletonID = sbx::k_DiffusionSkeletonID;
	request.m_ControlStrength = 1.0f;
	// Generate ONLY the character (txt2img + pose + LoRA) on a flat green background, then key the green
	// out and composite over the real living-room photo. The background stays pixel-exact (the original
	// photo) and the character brings no room with it — no regenerated-background overlap.
	request.m_BackgroundImageID = sbx::k_LivingRoomImageID;
	request.m_ChromaKeyThreshold = 0.5f;
	accessor.EmplaceOneframe<pg::GenerateImageRequestOneFrameComponent>(accessor.Create(), std::move(request));
}
