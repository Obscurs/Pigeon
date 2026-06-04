#include "pch.h"
#include "GraphicsContext.h"

#include "Pigeon/Renderer/Renderer.h"

#include "Platform/Windows/WindowsWindow.h"
#include "Platform/DirectX11/Dx11Context.h"

pg::S_Ptr<pg::GraphicsContext> pg::GraphicsContext::Create(const pg::WindowsWindow* window)
{
	switch (pg::Renderer::GetAPI())
	{
	case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::Dx11Context>(static_cast<HWND>(window->GetNativeWindow()));
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}