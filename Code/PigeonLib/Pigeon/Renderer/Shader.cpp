#include "pch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Shader.h"

namespace pigeon 
{
	Shader* Shader::Create(const char* vertexSrc, const char* fragmentSrc, const BufferLayout& buffLayout)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::DirectX11:  return new Dx11Shader(vertexSrc, fragmentSrc, buffLayout);
		}
		PG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}