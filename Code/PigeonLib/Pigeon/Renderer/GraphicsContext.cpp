#include "pch.h"
#include "GraphicsContext.h"

#include "Pigeon/Renderer/Renderer.h"

#include "Platform/Windows/WindowsWindow.h"
#include "Platform/DirectX11/Dx11Context.h"

pig::S_Ptr<pig::GraphicsContext> pig::GraphicsContext::Create(const pig::WindowsWindow* window)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return std::make_shared<pig::Dx11Context>(static_cast<HWND>(window->GetNativeWindow()));
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}