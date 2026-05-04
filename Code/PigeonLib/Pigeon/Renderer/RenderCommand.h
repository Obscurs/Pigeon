#pragma once

#include "RendererAPI.h"

namespace pig 
{
	class RenderCommand
	{
	public:
#ifdef TESTS_ENABLED
		static const pig::S_Ptr<RendererAPI> GetRenderAPI() { return s_RendererAPI; }
#endif

		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void Begin()
		{
			s_RendererAPI->Begin();
		}

		inline static void End()
		{
			s_RendererAPI->End();
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(unsigned int count)
		{
			s_RendererAPI->DrawIndexed(count);
		}
	private:
		static pig::S_Ptr<RendererAPI> s_RendererAPI;
	};

}