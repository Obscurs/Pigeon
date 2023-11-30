#include "pch.h"
#include "RenderCommand.h"

#include "Platform\DirectX11/Dx11RendererAPI.h"

namespace pigeon 
{
	RendererAPI* RenderCommand::s_RendererAPI = new Dx11RendererAPI;
}