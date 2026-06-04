#include "pch.h"
#include "RendererAPI.h"
#ifdef TESTS_ENABLED
pg::RendererAPI::API pg::RendererAPI::s_API = pg::RendererAPI::API::Testing;
#else
pg::RendererAPI::API pg::RendererAPI::s_API = pg::RendererAPI::API::DirectX11;
#endif