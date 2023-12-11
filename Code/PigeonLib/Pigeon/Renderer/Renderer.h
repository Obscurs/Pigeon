#pragma once

#include "RenderCommand.h"

namespace pigeon 
{
	class Renderer
	{
	public:
		static void BeginScene();
		static void EndScene();

		static void Submit(unsigned int count);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};


}