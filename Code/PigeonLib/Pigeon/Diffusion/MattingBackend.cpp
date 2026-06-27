#include "pch.h"
#include "Pigeon/Diffusion/MattingBackend.h"

#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/RendererAPI.h"
#include "Platform/Onnx/OnnxMattingBackend.h"
#include "Platform/Testing/TestingMattingBackend.h"

pg::S_Ptr<pg::MattingBackend> pg::MattingBackend::Create()
{
	switch (pg::Renderer::GetAPI())
	{
		case pg::RendererAPI::API::None:      PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case pg::RendererAPI::API::DirectX11: return std::make_shared<pg::OnnxMattingBackend>();
		case pg::RendererAPI::API::Testing:   return std::make_shared<pg::TestingMattingBackend>();
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}
