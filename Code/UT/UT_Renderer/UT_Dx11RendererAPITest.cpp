#pragma once
#include <catch.hpp>
#include <cstdlib>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Utils/TestApp.h"

#include <Pigeon/Renderer/OrthographicCamera.h>
#include <Pigeon/Renderer/Renderer2D.h>
#include <Pigeon/Renderer/Texture.h>

#include <Platform/DirectX11/Dx11Buffer.h>
#include <Platform/DirectX11/Dx11Context.h>
#include <Platform/DirectX11/Dx11RendererAPI.h>
#include <Platform/DirectX11/Dx11Shader.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

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

	char* s_Layout =
		"type Float3 POSITION\n"
		"type Float4 COLOR\n";

	float s_OurVertices[3 * 7] = {
		 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 0.45f, -0.5, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 -0.45f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
	};

	uint32_t s_Indices[3] = { 0, 1, 2 };

	glm::mat4 s_TestMat({
		1,  2,  3,  4,
		5,  6,  7,  8,
		9,  10, 11, 12,
		13, 14, 15, 16
	});

	glm::mat4 s_TestMat3({
		1,  2,  3,  
		4,  5,  6,  
		7,  8,  9
	});

	glm::mat4 s_TestMatTrnas({
		1,  5,  9,  13,
		2,  6,  10,  8,
		3,  7,  11, 15,
		4,  14, 12, 16
	});
	glm::mat4 s_TestMatIdent({
		1,  0,  0,  0,
		0,  1,  0,  0,
		0,  0,  1,  0,
		0,  0,  0,  1
	});

	glm::vec2 s_TestVec2(1.f);
	glm::vec3 s_TestVec3(1.f);
	glm::vec4 s_TestVec4(1.f);
	float s_TestFloat = 1.f;
	int s_TestInt = 1;

	struct CBData0
	{
		float value1;              // 4 bytes
		float value2;              // 4 bytes
		float value3;              // 4 bytes
		float value4;              // 4 bytes
	};

	struct CBData1
	{
		DirectX::XMMATRIX matrix; // 16-byte aligned
	};
	struct CBData2
	{
		DirectX::XMMATRIX matrix; // 16-byte aligned
		DirectX::XMFLOAT4 vector;  // 16-byte aligned (padded vec3)
		float value1;              // 4 bytes
		float value2;              // 4 bytes
		float padding[2];          // Padding for alignment - 8 bytes
	};

	struct CBDataInvalid
	{
		float value1;              // 4 bytes
		float value2;              // 4 bytes
	};
}

namespace CatchTestsetFail
{
	TEST_CASE("app.Renderer::Dx11RendererAPITest")
	{
		pig::Application& app = pig::CreateApplication();

		const pig::S_Ptr<pig::Dx11RendererAPI> rendererAPI = std::dynamic_pointer_cast<pig::Dx11RendererAPI>(pig::RenderCommand::GetRenderAPI());
		CHECK(rendererAPI->GetAPI() == pig::RendererAPI::API::DirectX11);
		CHECK(!rendererAPI->GetData().m_Initialized); //Not initialized until first frame;
		CHECK(rendererAPI->GetData().m_MainRenderTargetView == nullptr);
		CHECK(rendererAPI->GetData().m_ClearColor[0] == 1.f);
		CHECK(rendererAPI->GetData().m_ClearColor[1] == 1.f);
		CHECK(rendererAPI->GetData().m_ClearColor[2] == 1.f);
		CHECK(rendererAPI->GetData().m_ClearColor[3] == 1.f);

		pig::RenderCommand::SetClearColor({ 0.1f, 0.2f, 0.3f, 0.4f });

		CHECK(rendererAPI->GetData().m_ClearColor[0] == 0.1f);
		CHECK(rendererAPI->GetData().m_ClearColor[1] == 0.2f);
		CHECK(rendererAPI->GetData().m_ClearColor[2] == 0.3f);
		CHECK(rendererAPI->GetData().m_ClearColor[3] == 0.4f);

		pig::Renderer::BeginScene();

		CHECK(rendererAPI->GetData().m_Initialized);
		CHECK(rendererAPI->GetData().m_MainRenderTargetView != nullptr);
		pig::Renderer::Submit(0);
		pig::Renderer::EndScene();
	}

	TEST_CASE("Renderer::Renderer2D")
	{
		pig::Application& app = pig::CreateApplication();

		const pig::S_Ptr<pig::Dx11RendererAPI> rendererAPI = std::dynamic_pointer_cast<pig::Dx11RendererAPI>(pig::RenderCommand::GetRenderAPI());
		pig::OrthographicCameraController cameraController(1280.0f / 720.0f);
		
		SECTION("Empty call")
		{
			const pig::Renderer2D::Data& data = pig::Renderer2D::GetData();
			CHECK(!data.m_Camera);
			CHECK(data.m_Shader);
			CHECK(data.m_VertexBuffer);
			CHECK(data.m_IndexBuffer);
			CHECK(data.m_WhiteTexture);

			pig::Renderer2D::Clear({0.f, 0.f, 0.f, 1.f});
			pig::Renderer2D::BeginScene(cameraController);
			CHECK(data.m_Camera);
			pig::Renderer2D::EndScene();
			CHECK(!data.m_Camera);
		}
		SECTION("Draw quad")
		{
			pig::Renderer2D::BeginScene(cameraController);

			glm::vec3 pos(4.f, 5.f, 6.f);
			glm::vec3 col(7.f, 8.f, 9.f);
			glm::vec3 scale(1.f, 2.f, 3.f);
			pig::Renderer2D::DrawQuad(pos, scale, col);

			pig::Renderer2D::EndScene();
		}
		SECTION("Draw textured quad")
		{
			pig::U_Ptr<pig::Texture2D> texture = pig::Texture2D::Create("Assets/Test/SampleTexture.png");

			pig::Renderer2D::BeginScene(cameraController);

			glm::vec3 pos(4.f, 5.f, 6.f);
			glm::vec3 col(7.f, 8.f, 9.f);
			glm::vec3 scale(1.f, 2.f, 3.f);
			pig::Renderer2D::DrawQuad(pos, scale, *texture);

			pig::Renderer2D::EndScene();
		}
		SECTION("Batch rendering")
		{
			const pig::Renderer2D::Data& data = pig::Renderer2D::GetData();
			
			glm::vec3 pos(4.f, 5.f, 6.f);
			glm::vec3 col(7.f, 8.f, 9.f);
			glm::vec3 scale(1.f, 2.f, 3.f);

			pig::Renderer2D::Clear({ 0.f, 0.f, 0.f, 1.f });
			pig::Renderer2D::BeginScene(cameraController);
			pig::Renderer2D::DrawQuad(pos, scale, col);
			CHECK(data.m_VertexCount == 4);
			
			pig::Renderer2D::EndScene();
		}
	}

	TEST_CASE("app.Renderer::Dx11ContextTest")
	{
		pig::Application& app = pig::CreateApplication();

		pig::Dx11Context* dx11Context = static_cast<pig::Dx11Context*>(app.GetWindow().GetGraphicsContext());
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
		pig::Renderer::BeginScene();
		CHECK(!dx11Context->NeedsResize());
		CHECK(dx11Context->GetHeight() == 456);
		CHECK(dx11Context->GetWidth() == 123);

		dx11Context->Shutdown();
		CHECK(dx11Context->GetData().m_HWnd == nullptr);
		CHECK(dx11Context->GetPd3dDevice() == nullptr);
		CHECK(dx11Context->GetPd3dDeviceContext() == nullptr);
		CHECK(dx11Context->GetSwapChain() == nullptr);
	}

	TEST_CASE("app.Renderer::Dx11Buffers")
	{
		pig::Application& app = pig::CreateApplication();

		pig::Dx11Context* dx11Context = static_cast<pig::Dx11Context*>(app.GetWindow().GetGraphicsContext());

		SECTION("Vertex buffer")
		{
			pig::U_Ptr<pig::VertexBuffer> vertexBuffer = std::move(pig::VertexBuffer::Create(s_OurVertices, sizeof(s_OurVertices), sizeof(float) * 7));
			pig::Dx11VertexBuffer* dxVB = static_cast<pig::Dx11VertexBuffer*>(vertexBuffer.get());
			CHECK(dxVB->GetData().m_Buffer != nullptr);

			SECTION("Append vertices")
			{
				vertexBuffer->AppendVertices(s_OurVertices, 3, 0);
			}

			dxVB->Bind();
			dxVB->Unbind();
		}
		SECTION("Index buffer")
		{
			pig::U_Ptr<pig::IndexBuffer> indexBuffer = std::move(pig::IndexBuffer::Create(s_Indices, sizeof(s_Indices) / sizeof(uint32_t)));
			pig::Dx11IndexBuffer* dxIB = static_cast<pig::Dx11IndexBuffer*>(indexBuffer.get());
			CHECK(dxIB->GetData().m_Buffer != nullptr);
			CHECK(dxIB->GetCount() == 3);

			SECTION("Append indices")
			{
				indexBuffer->AppendIndices(s_Indices, 3, 0);
			}

			dxIB->Bind();
			dxIB->Unbind();
		}
		SECTION("Constant buffer")
		{
			CBDataInvalid dataInvalid;
			pig::U_Ptr<pig::Dx11ConstantBuffer<CBDataInvalid>> constantBufferInvalid = std::make_unique<pig::Dx11ConstantBuffer<CBDataInvalid>>(&dataInvalid);
			CHECK(constantBufferInvalid->GetData().m_Buffer == nullptr); //Error creating, invalid alignment

			CBData0 data;
			pig::U_Ptr<pig::Dx11ConstantBuffer<CBData0>> constantBuffer = std::make_unique<pig::Dx11ConstantBuffer<CBData0>>(&data);
			CHECK(constantBuffer->GetData().m_Buffer != nullptr);
			constantBuffer->Bind(0);

			CBData1 data1;
			pig::U_Ptr<pig::Dx11ConstantBuffer<CBData1>> constantBuffer1 =std::make_unique<pig::Dx11ConstantBuffer<CBData1>>(&data1);
			CHECK(constantBuffer1->GetData().m_Buffer != nullptr);
			constantBuffer1->Bind(0);

			CBData2 data2;
			pig::U_Ptr<pig::Dx11ConstantBuffer<CBData2>> constantBuffer2 = std::make_unique<pig::Dx11ConstantBuffer<CBData2>>(&data2);
			CHECK(constantBuffer2->GetData().m_Buffer != nullptr);
			constantBuffer2->Bind(0);
		}
	}

	TEST_CASE("app.Renderer::Dx11Shader")
	{
		pig::Application& app = pig::CreateApplication();

		pig::Dx11Context* dx11Context = static_cast<pig::Dx11Context*>(app.GetWindow().GetGraphicsContext());
		
		pig::BufferLayout buffLayout;
		pig::S_Ptr<pig::Shader> shader;

		pig::ShaderLibrary shaderLibrary;

		SECTION("position and color")
		{
			shader = std::move(pig::Shader::Create("test", s_VsCode, s_PsCode, s_Layout));
		}

		SECTION("position and texture from file")
		{
			shader = std::move(pig::Shader::Create("Assets/Test/UTTestShader.shader"));
		}

		SECTION("position and texture from file using shaderlib")
		{
			shader = std::move(shaderLibrary.Load("Assets/Test/UTTestShader.shader"));

			shader = shaderLibrary.Get("UTTestShader");
		}

		pig::Dx11Shader* dxShader = static_cast<pig::Dx11Shader*>(shader.get());

		CHECK(dxShader->GetData().m_InputLayout != nullptr);
		CHECK(dxShader->GetData().m_PixelShader != nullptr);
		CHECK(dxShader->GetData().m_VertexShader != nullptr);
		dxShader->Bind();
		shader->UploadUniformMat4("u_ViewProjection", s_TestMat);
		shader->UploadUniformMat4("u_Transform", s_TestMat);
		shader->UploadUniformMat3("u_Color", s_TestMat3);
		shader->UploadUniformInt("u_Color", s_TestInt);
		shader->UploadUniformFloat("u_Color", s_TestFloat);
		shader->UploadUniformFloat2("u_Color", s_TestVec2);
		shader->UploadUniformFloat3("u_Color", s_TestVec3);
		shader->UploadUniformFloat4("u_Color", s_TestVec4);

		dxShader->Unbind();
	}

	TEST_CASE("Renderer::Texture")
	{
		pig::Application& app = pig::CreateApplication();

		SECTION("File Texture")
		{
			pig::U_Ptr<pig::Texture2D> texture = pig::Texture2D::Create("Assets/Test/SampleTexture.png");
			texture->Bind(0);
			CHECK(texture->GetHeight() == 64);
			CHECK(texture->GetWidth() == 64);
		}
		SECTION("Data Texture")
		{
			const unsigned int width = 128;
			const unsigned int height = 64;
			SECTION("RGB")
			{
				const unsigned int channels = 3;
				std::vector<unsigned char> data(width * height * channels, 255);
				pig::U_Ptr<pig::Texture2D> texture = pig::Texture2D::Create(width, height, channels, data.data());
				texture->Bind(0);
				CHECK(texture->GetHeight() == 64);
				CHECK(texture->GetWidth() == 128);
			}
			SECTION("RGBA")
			{
				const unsigned int channels = 4;
				std::vector<unsigned char> data(width * height * channels, 255);
				pig::U_Ptr<pig::Texture2D> texture = pig::Texture2D::Create(width, height, channels, data.data());
				texture->Bind(0);
				CHECK(texture->GetHeight() == 64);
				CHECK(texture->GetWidth() == 128);
			}
		}
	}

	TEST_CASE("Renderer::OrthographicCamera")
	{
		pig::Application& app = pig::CreateApplication();
		const glm::vec4 ortoValues(-0.5f, 0.5f, -0.5f, 0.5f);
		const glm::vec3 pos0(0.f, 0.f, 0.f);
		const glm::vec3 pos1(-1.f, 2.f, 3.f);
		const glm::vec3 pos2(245.f, 672.6f, 12.341f);
		const float rot0 = 0.f;
		const float rot1 = 1.5f;
		const float rot2 = -4.5f;

		pig::OrthographicCamera camera(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w);
		CHECK(camera.GetPosition() == pos0);
		CHECK(camera.GetRotation() == rot0);
		glm::mat4 projMat = camera.GetProjectionMatrix();
		glm::mat4 viewMat = camera.GetViewMatrix();
		glm::mat4 projViewMat = camera.GetViewProjectionMatrix();

		CHECK(projMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f));
		CHECK(viewMat == glm::mat4(1.f));
		CHECK(projViewMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f));

		camera.SetPosition(pos1);
		CHECK(camera.GetPosition() == pos1);
		CHECK(camera.GetRotation() == rot0);
		camera.SetRotation(rot1);
		CHECK(camera.GetPosition() == pos1);
		CHECK(camera.GetRotation() == rot1);

		projMat = camera.GetProjectionMatrix();
		viewMat = camera.GetViewMatrix();
		projViewMat = camera.GetViewProjectionMatrix();

		glm::mat4 inverseTransform = glm::inverse(glm::translate(glm::mat4(1.0f), pos1) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rot1), glm::vec3(0, 0, 1)));

		CHECK(projMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f));
		CHECK(viewMat == inverseTransform);
		CHECK(projViewMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f) * inverseTransform);

		camera.SetRotation(rot2);
		CHECK(camera.GetPosition() == pos1);
		CHECK(camera.GetRotation() == rot2);
		camera.SetPosition(pos2);
		CHECK(camera.GetPosition() == pos2);
		CHECK(camera.GetRotation() == rot2);

		projMat = camera.GetProjectionMatrix();
		viewMat = camera.GetViewMatrix();
		projViewMat = camera.GetViewProjectionMatrix();

		inverseTransform = glm::inverse(glm::translate(glm::mat4(1.0f), pos2) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rot2), glm::vec3(0, 0, 1)));

		CHECK(projMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f));
		CHECK(viewMat == inverseTransform);
		CHECK(projViewMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f) * inverseTransform);
	}
	TEST_CASE("Renderer::OrthographicCameraController")
	{
		pig::Application& app = pig::CreateApplication();
		SECTION("Keyboard events")
		{
			//UNTESTED FOR NOW, do not have any input events helper
		}
		SECTION("Mouse events")
		{
			SECTION("Mouse scrolled")
			{
				pig::OrthographicCameraController cameraController(0.5f);

				cameraController.SetZoomLevel(3.5f);
				CHECK(cameraController.GetZoomLevel() == 3.5f);

				CHECK(cameraController.GetData().m_AspectRatio == 0.5f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 5.f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				cameraController.OnUpdate(5);

				CHECK(cameraController.GetData().m_AspectRatio == 0.5f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 3.5f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				const glm::vec3 pos0(0.f, 0.f, 0.f);
				const float rot0 = 0.f;

				pig::MouseScrolledEvent event(123.f, 456.f);

				cameraController.OnEvent(event);
				cameraController.OnUpdate(5);
				
				CHECK(cameraController.GetCamera().GetPosition() == pos0);
				CHECK(cameraController.GetCamera().GetRotation() == rot0);
				CHECK(cameraController.GetZoomLevel() == 0.25f);
				CHECK(cameraController.GetData().m_AspectRatio == 0.5f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 0.25f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				pig::OrthographicCamera cameraTest(-0.125f, 0.125f, -0.25f, 0.25f);
				const pig::OrthographicCamera& controllerCamera = cameraController.GetCamera();
				CHECK(cameraTest.GetData().m_Position == controllerCamera.GetData().m_Position);
				CHECK(cameraTest.GetData().m_Rotation == controllerCamera.GetData().m_Rotation);
				CHECK(cameraTest.GetData().m_ViewMatrix == controllerCamera.GetData().m_ViewMatrix);
				CHECK(cameraTest.GetData().m_ViewProjectionMatrix == controllerCamera.GetData().m_ViewProjectionMatrix);
				CHECK(cameraTest.GetData().m_ProjectionMatrix == controllerCamera.GetData().m_ProjectionMatrix);
			}
			SECTION("Window Resized")
			{
				pig::OrthographicCameraController cameraController(0.123f);
				CHECK(cameraController.GetZoomLevel() == 1.0f);
				cameraController.SetZoomLevel(4.f);

				pig::WindowResizeEvent event(100.f, 1000.f);
				cameraController.OnEvent(event);
				cameraController.OnUpdate(10);

				const glm::vec3 pos0(0.f, 0.f, 0.f);
				const float rot0 = 0.f;

				CHECK(cameraController.GetCamera().GetPosition() == pos0);
				CHECK(cameraController.GetCamera().GetRotation() == rot0);
				CHECK(cameraController.GetZoomLevel() == 4.f);
				CHECK(cameraController.GetData().m_AspectRatio == 0.1f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 4.f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				pig::OrthographicCamera cameraTest(-0.4f, 0.4f, -4.f, 4.f);
				const pig::OrthographicCamera& controllerCamera = cameraController.GetCamera();
				CHECK(cameraTest.GetData().m_Position == controllerCamera.GetData().m_Position);
				CHECK(cameraTest.GetData().m_Rotation == controllerCamera.GetData().m_Rotation);
				CHECK(cameraTest.GetData().m_ViewMatrix == controllerCamera.GetData().m_ViewMatrix);
				CHECK(cameraTest.GetData().m_ViewProjectionMatrix == controllerCamera.GetData().m_ViewProjectionMatrix);
				CHECK(cameraTest.GetData().m_ProjectionMatrix == controllerCamera.GetData().m_ProjectionMatrix);
			}
		}
	}
} // End namespace: CatchTestsetFail

