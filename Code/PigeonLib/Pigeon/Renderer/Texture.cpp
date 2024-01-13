#include "pch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/DirectX11/Dx11Texture.h"

pig::U_Ptr<pig::Texture2D> pig::Texture2D::Create(const std::string& path)
{
	switch (pig::Renderer::GetAPI())
	{
		case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case pig::RendererAPI::API::DirectX11:  return std::make_unique<pig::Dx11Texture2D>(path);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}