#pragma once

#include "Pigeon/Core/UUID.h"
#include "Pigeon/Diffusion/Image.h"

namespace pg
{
	// Emitted by DiffusionSystem when a Diffusion Job completes; drained by ResourceManagerSystem (the
	// sole writer of the resource map), which registers m_Image as a Texture2D under m_TextureID in the
	// texture map so it is sampled like any loaded texture.
	struct RegisterGeneratedTextureRequestOneFrameComponent
	{
		pg::UUID m_TextureID;
		pg::Image m_Image;
	};
}
