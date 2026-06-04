#include "pch.h"
#include "RenderCommand.h"


#ifdef TESTS_ENABLED
#include "Platform\Testing/TestingRendererAPI.h"
pg::S_Ptr<pg::RendererAPI> pg::RenderCommand::s_RendererAPI = std::make_shared<pg::TestingRendererAPI>();
#else
#include "Platform\DirectX11/Dx11RendererAPI.h"
pg::S_Ptr<pg::RendererAPI> pg::RenderCommand::s_RendererAPI = std::make_shared<pg::Dx11RendererAPI>();
#endif