#include "pch.h"
#include "RenderCommand.h"


#ifdef TESTS_ENABLED
#include "Platform\Testing/TestingRendererAPI.h"
pig::S_Ptr<pig::RendererAPI> pig::RenderCommand::s_RendererAPI = std::make_shared<pig::TestingRendererAPI>();
#else
#include "Platform\DirectX11/Dx11RendererAPI.h"
pig::S_Ptr<pig::RendererAPI> pig::RenderCommand::s_RendererAPI = std::make_shared<pig::Dx11RendererAPI>();
#endif