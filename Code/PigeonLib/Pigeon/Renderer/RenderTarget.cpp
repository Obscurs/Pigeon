#include "pch.h"
#include "Pigeon/Renderer/RenderTarget.h"

#include "Pigeon/Renderer/Renderer.h"
#include "Platform/DirectX11/Dx11RenderTarget.h"
#include "Platform/Testing/TestingRenderTarget.h"

pg::S_Ptr<pg::RenderTarget> pg::RenderTarget::Create(uint32_t width, uint32_t height)
{
	switch (pg::Renderer::GetAPI())
	{
		case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::Dx11RenderTarget>(width, height);
		case pg::RendererAPI::API::Testing:  return std::make_shared<pg::TestingRenderTarget>(width, height);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}
