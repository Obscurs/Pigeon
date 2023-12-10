#pragma once
#include <catch.hpp>
#include <cstdlib>

#include "Utils/TestApp.h"

#include <Platform/DirectX11/Dx11Buffer.h>
#include <Platform/DirectX11/Dx11Context.h>
#include <Platform/DirectX11/Dx11RendererAPI.h>
#include <Platform/DirectX11/Dx11Shader.h>

namespace
{
	// Simple vertex shader
	char* s_VsCode =
		"struct VS_INPUT\n"
		"{\n"
		"	float4 Pos : POSITION;\n"
		"	float4 Col : COLOR;\n"
		"};\n"
		"struct PS_INPUT\n"
		"{\n"
		"	float4 Pos : SV_POSITION; \n"
		"	float4 Col : COLOR; \n"
		"};\n"
		"PS_INPUT main(VS_INPUT input)\n"
		"{\n"
		"	PS_INPUT output;\n"
		"	output.Pos = input.Pos; // Pass position to rasterizer\n"
		"	output.Col = input.Col; // Pass color to pixel shader\n"
		"	return output;\n"
		"};";

	// Simple pixel shader
	char* s_PsCode =
		"struct PS_INPUT\n"
		"{\n"
		"	float4 Pos : SV_POSITION;\n"
		"	float4 Col : COLOR;\n"
		"};\n"
		"float4 main(PS_INPUT input) : SV_TARGET\n"
		"{\n"
		"    return input.Col; // Output the interpolated color\n"
		"};";


	float s_OurVertices[3 * 7] = {
		 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 0.45f, -0.5, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 -0.45f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
	};

	uint32_t s_Indices[3] = { 0, 1, 2 };
}

namespace CatchTestsetFail
{
	TEST_CASE("app.Renderer::Dx11RendererAPITest")
	{
		TestApp* app = static_cast<TestApp*>(pigeon::CreateApplication());

		const pigeon::Dx11RendererAPI* rendererAPI = static_cast<const pigeon::Dx11RendererAPI*>(pigeon::RenderCommand::GetRenderAPI());
		CHECK(rendererAPI->GetAPI() == pigeon::RendererAPI::API::DirectX11);
		CHECK(!rendererAPI->GetData().m_Initialized); //Not initialized until first frame;
		CHECK(rendererAPI->GetData().m_MainRenderTargetView == nullptr);
		CHECK(rendererAPI->GetData().m_ClearColor[0] == 1.f);
		CHECK(rendererAPI->GetData().m_ClearColor[1] == 1.f);
		CHECK(rendererAPI->GetData().m_ClearColor[2] == 1.f);
		CHECK(rendererAPI->GetData().m_ClearColor[3] == 1.f);

		pigeon::RenderCommand::SetClearColor({ 0.1f, 0.2f, 0.3f, 0.4f });

		CHECK(rendererAPI->GetData().m_ClearColor[0] == 0.1f);
		CHECK(rendererAPI->GetData().m_ClearColor[1] == 0.2f);
		CHECK(rendererAPI->GetData().m_ClearColor[2] == 0.3f);
		CHECK(rendererAPI->GetData().m_ClearColor[3] == 0.4f);

		pigeon::Renderer::BeginScene();

		CHECK(rendererAPI->GetData().m_Initialized);
		CHECK(rendererAPI->GetData().m_MainRenderTargetView != nullptr);
		pigeon::Renderer::Submit();
		pigeon::Renderer::EndScene();

		delete app;
	}

	TEST_CASE("app.Renderer::Dx11ContextTest")
	{
		TestApp* app = static_cast<TestApp*>(pigeon::CreateApplication());

		pigeon::Dx11Context* dx11Context = static_cast<pigeon::Dx11Context*>(app->GetWindow().GetGraphicsContext());
		CHECK(dx11Context->GetData().m_HWnd != nullptr);
		CHECK(dx11Context->GetHeight() == 720);
		CHECK(dx11Context->GetWidth() == 1280);
		CHECK(dx11Context->GetPd3dDevice() != nullptr);
		CHECK(dx11Context->GetPd3dDeviceContext() != nullptr);
		CHECK(dx11Context->GetSwapChain() != nullptr);
		CHECK(!dx11Context->NeedsResize());

		dx11Context->SetSize(123, 456);
		CHECK(dx11Context->NeedsResize());
		CHECK(dx11Context->GetHeight() == 720);
		CHECK(dx11Context->GetWidth() == 1280);
		pigeon::Renderer::BeginScene();
		CHECK(!dx11Context->NeedsResize());
		CHECK(dx11Context->GetHeight() == 456);
		CHECK(dx11Context->GetWidth() == 123);

		dx11Context->Shutdown();
		CHECK(dx11Context->GetData().m_HWnd == nullptr);
		CHECK(dx11Context->GetPd3dDevice() == nullptr);
		CHECK(dx11Context->GetPd3dDeviceContext() == nullptr);
		CHECK(dx11Context->GetSwapChain() == nullptr);
		delete app;
	}

	TEST_CASE("app.Renderer::Dx11Buffers")
	{
		TestApp* app = static_cast<TestApp*>(pigeon::CreateApplication());

		pigeon::Dx11Context* dx11Context = static_cast<pigeon::Dx11Context*>(app->GetWindow().GetGraphicsContext());


		SECTION("Vertex buffer")
		{
			pigeon::VertexBuffer* vertexBuffer = pigeon::VertexBuffer::Create(s_OurVertices, sizeof(s_OurVertices));
			pigeon::Dx11VertexBuffer* dxVB = static_cast<pigeon::Dx11VertexBuffer*>(vertexBuffer);
			CHECK(dxVB->GetData().m_Buffer != nullptr);
			dxVB->Bind();
			dxVB->Unbind();
		}
		SECTION("Index buffer")
		{
			pigeon::IndexBuffer* indexBuffer = pigeon::IndexBuffer::Create(s_Indices, sizeof(s_Indices) / sizeof(uint32_t));
			pigeon::Dx11IndexBuffer* dxIB = static_cast<pigeon::Dx11IndexBuffer*>(indexBuffer);
			CHECK(dxIB->GetData().m_Buffer != nullptr);
			CHECK(dxIB->GetCount() == 3);
			dxIB->Bind();
			dxIB->Unbind();
		}

		delete app;
	}

	TEST_CASE("app.Renderer::Dx11Shader")
	{
		TestApp* app = static_cast<TestApp*>(pigeon::CreateApplication());

		pigeon::Dx11Context* dx11Context = static_cast<pigeon::Dx11Context*>(app->GetWindow().GetGraphicsContext());

		pigeon::Shader* shader;

		pigeon::BufferLayout buffLayout = {
			{ pigeon::ShaderDataType::Float3, "POSITION" },
			{ pigeon::ShaderDataType::Float4, "COLOR" }
		};

		shader = pigeon::Shader::Create(s_VsCode, s_PsCode, buffLayout);
		pigeon::Dx11Shader* dxShader = static_cast<pigeon::Dx11Shader*>(shader);

		CHECK(dxShader->GetData().m_InputLayout != nullptr);
		CHECK(dxShader->GetData().m_PixelShader != nullptr);
		CHECK(dxShader->GetData().m_VertexShader != nullptr);
		dxShader->Bind();
		dxShader->Unbind();

		delete app;
	}
} // End namespace: CatchTestsetFail

