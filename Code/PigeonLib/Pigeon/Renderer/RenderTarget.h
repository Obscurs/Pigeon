#pragma once

#include "Pigeon/Core/Core.h"
#include "Pigeon/Renderer/Texture.h"

namespace pg
{
	// A platform-abstracted offscreen draw surface: a colour texture plus a depth buffer that geometry
	// is rendered into instead of the window back buffer. Created by ResourceManagerSystem from a
	// render-target manifest entry; the 3D pass draws into it (via RenderCommand::BeginRenderTarget),
	// and its colour buffer is registered in the resource map's texture map so 2D draws can sample the
	// result. The DirectX11 backend owns real GPU surfaces; the Testing backend is a no-op.
	class RenderTarget
	{
	public:
		virtual ~RenderTarget() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// The colour buffer, usable as an ordinary sampled texture.
		virtual pg::S_Ptr<pg::Texture2D> GetColorTexture() const = 0;

		static pg::S_Ptr<RenderTarget> Create(uint32_t width, uint32_t height);
	};
}
