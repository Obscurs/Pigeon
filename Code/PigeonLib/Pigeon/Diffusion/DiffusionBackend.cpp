#include "pch.h"
#include "Pigeon/Diffusion/DiffusionBackend.h"

#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/RendererAPI.h"
#include "Platform/StableDiffusion/StableDiffusionCppBackend.h"
#include "Platform/Testing/TestingDiffusionBackend.h"

pg::S_Ptr<pg::DiffusionBackend> pg::DiffusionBackend::Create()
{
	switch (pg::Renderer::GetAPI())
	{
		case pg::RendererAPI::API::None:      PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case pg::RendererAPI::API::DirectX11: return std::make_shared<pg::StableDiffusionCppBackend>();
		case pg::RendererAPI::API::Testing:   return std::make_shared<pg::TestingDiffusionBackend>();
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}
