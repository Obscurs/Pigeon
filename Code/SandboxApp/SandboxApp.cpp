#include <Pigeon.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"

#include "Pigeon/Renderer/OrthographicCamera.h"
namespace
{
	char* s_VsCode =
		"	cbuffer MatrixBuffer : register(b0)\n"
		"{ \n"
		"	matrix u_ViewProjection; \n"
		"}; \n"

		"	cbuffer MatrixBuffer : register(b1)\n"
		"{ \n"
		"	matrix u_Transform; \n"
		"}; \n"

		"	cbuffer VectorBuffer : register(b2)\n"
		"{ \n"
		"	float3 u_Color; \n"
		"   float padding; \n"
		"}; \n"

		"struct VS_INPUT\n"
		"{\n"
		"	float3 Pos : POSITION;\n"
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
		"	output.Pos = mul(float4(input.Pos, 1.f), u_Transform); // Pass position to rasterizer\n"
		"	output.Pos = mul(output.Pos, u_ViewProjection); // Pass position to rasterizer\n"
		"	output.Col = float4(u_Color.x, u_Color.y, u_Color.z, 1.0f); // Pass color to pixel shader\n"
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

	class ExampleLayer : public pigeon::Layer
	{
	public:
		ExampleLayer()
			: Layer("Example"),
			m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
		{
			m_VertexBuffer.reset(pigeon::VertexBuffer::Create(s_OurVertices, sizeof(s_OurVertices)));
			m_IndexBuffer.reset(pigeon::IndexBuffer::Create(s_Indices, sizeof(s_Indices) / sizeof(uint32_t)));

			pigeon::BufferLayout buffLayout = {
				{ pigeon::ShaderDataType::Float3, "POSITION" },
				{ pigeon::ShaderDataType::Float4, "COLOR" }
			};

			m_Shader.reset(pigeon::Shader::Create(s_VsCode, s_PsCode, buffLayout));
		}

		~ExampleLayer()
		{
			m_Shader.reset();

			m_VertexBuffer.reset();
			m_IndexBuffer.reset();
		}

		void OnUpdate() override
		{
			pigeon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.f });

			m_Camera.SetPosition({ 0.5f, 0.5f, 0.0f });
			m_Camera.SetRotation(45.0f);

			pigeon::Renderer::BeginScene();
			m_SceneData.ViewProjectionMatrix = m_Camera.GetViewProjectionMatrix();

			m_VertexBuffer->Bind();
			m_IndexBuffer->Bind();
			m_Shader->Bind();
			m_Shader->UploadUniformMat4("u_ViewProjection", m_SceneData.ViewProjectionMatrix);
			m_Shader->UploadUniformFloat3("u_Color", m_SquareColor);

			glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
			for (int y = -10; y < 10; y++)
			{
				for (int x = -10; x < 10; x++)
				{
					glm::vec3 pos(x *0.11f, y * 0.11f, 0.f);
					glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
					m_Shader->UploadUniformMat4("u_Transform", transform);
					pigeon::Renderer::Submit(3);
				}
			}
			pigeon::Renderer::EndScene();

			//if (pigeon::Input::IsKeyPressed(PG_KEY_TAB))
			//	PG_TRACE("Tab key is pressed (poll)!");
		}

		virtual void OnImGuiRender() override
		{
			ImGui::Begin("Settings");
			ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
			ImGui::End();
		}

		void OnEvent(pigeon::Event& event) override
		{
			/*if (event.GetEventType() == pigeon::EventType::KeyPressed)
			{
				pigeon::KeyPressedEvent& e = (pigeon::KeyPressedEvent&)event;
				if (e.GetKeyCode() == PG_KEY_TAB)
					PG_TRACE("Tab key is pressed (event)!");
				PG_TRACE("{0}", (char)e.GetKeyCode());
			}*/
		}

	private:
		std::unique_ptr<pigeon::VertexBuffer> m_VertexBuffer;
		std::unique_ptr<pigeon::IndexBuffer> m_IndexBuffer;
		std::unique_ptr<pigeon::Shader> m_Shader;

		pigeon::OrthographicCamera m_Camera;
		glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		SceneData m_SceneData;
	};
}

class Sandbox : public pigeon::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}
};

pigeon::Application* pigeon::CreateApplication()
{
	return new Sandbox();
}