#pragma once

#include "RendererAPI.h"

namespace pigeon 
{
	class RenderCommand
	{
	public:
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

		inline static void DrawIndexed()
		{
			s_RendererAPI->DrawIndexed();
		}
	private:
		static RendererAPI* s_RendererAPI;
	};

}