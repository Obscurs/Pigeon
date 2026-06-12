#pragma once

#include <glm/glm.hpp>

namespace pg
{
	class RenderTarget;

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, DirectX11 = 1, Testing = 2
		};
	public:
		virtual void SetClearColor(const glm::vec4& color) = 0;

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		// Restricts subsequent draws to the given window-pixel rectangle (top-left origin). The renderer's
		// UI pass uses this to mask clipped sub-trees; the rect is reset to the full window each frame.
		virtual void SetScissor(int x, int y, int width, int height) = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Clear() = 0;

		// Binds the offscreen render target (colour + depth), sizes the viewport to it, enables depth
		// testing, and clears it. Subsequent DrawIndexed calls draw into the target until EndRenderTarget
		// restores the window back buffer and the 2D no-depth state.
		virtual void BeginRenderTarget(RenderTarget& target, const glm::vec4& clearColor) = 0;
		virtual void EndRenderTarget() = 0;

		virtual void DrawIndexed(unsigned int count) = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}