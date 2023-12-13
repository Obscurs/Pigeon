#include "pch.h"
#include "RenderCommand.h"

#include "Platform\DirectX11/Dx11RendererAPI.h"

pig::S_Ptr<pig::RendererAPI> pig::RenderCommand::s_RendererAPI = std::make_shared<pig::Dx11RendererAPI>();