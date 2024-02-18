#include "pch.h"
#include "RendererAPI.h"
#ifdef TESTS_ENABLED
pig::RendererAPI::API pig::RendererAPI::s_API = pig::RendererAPI::API::Testing;
#else
pig::RendererAPI::API pig::RendererAPI::s_API = pig::RendererAPI::API::DirectX11;
#endif