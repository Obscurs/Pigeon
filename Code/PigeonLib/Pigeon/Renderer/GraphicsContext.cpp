#include "pch.h"
#include "GraphicsContext.h"

#include "Pigeon/Renderer/Renderer.h"

#include "Platform/Windows/WindowsWindow.h"
#include "Platform/DirectX11/Dx11Context.h"

namespace pigeon
{
	GraphicsContext* GraphicsContext::Create(const WindowsWindow* window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::DirectX11:  return new Dx11Context(static_cast<HWND>(window->GetNativeWindow()));
		}

		PG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}