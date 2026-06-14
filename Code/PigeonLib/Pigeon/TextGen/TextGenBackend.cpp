#include "pch.h"
#include "Pigeon/TextGen/TextGenBackend.h"

#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/RendererAPI.h"
#include "Platform/Llama/LlamaCppBackend.h"
#include "Platform/Testing/TestingTextGenBackend.h"

pg::S_Ptr<pg::TextGenBackend> pg::TextGenBackend::Create()
{
	switch (pg::Renderer::GetAPI())
	{
		case pg::RendererAPI::API::None:      PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case pg::RendererAPI::API::DirectX11: return std::make_shared<pg::LlamaCppBackend>();
		case pg::RendererAPI::API::Testing:   return std::make_shared<pg::TestingTextGenBackend>();
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}
