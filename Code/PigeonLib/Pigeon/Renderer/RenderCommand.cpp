#include "pch.h"
#include "RenderCommand.h"

#include "Platform\DirectX11/Dx11RendererAPI.h"

pig::RendererAPI* pig::RenderCommand::s_RendererAPI = new pig::Dx11RendererAPI;