#include "pch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Shader.h"

pig::U_Ptr<pig::Shader> pig::Shader::Create(const char* vertexSrc, const char* fragmentSrc, const pig::BufferLayout& buffLayout)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return std::make_unique<pig::Dx11Shader>(vertexSrc, fragmentSrc, buffLayout);
	}
	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}