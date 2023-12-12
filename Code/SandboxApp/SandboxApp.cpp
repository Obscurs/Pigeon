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

	class ExampleLayer : public pig::Layer
	{
	public:
		ExampleLayer()
			: Layer("Example"),
			m_Camera(-1.6f, 1.6f, -0.9f, 0.9f),
			m_CameraPosition(0.0f)
		{
			m_VertexBuffer.reset(pig::VertexBuffer::Create(s_OurVertices, sizeof(s_OurVertices)));
			m_IndexBuffer.reset(pig::IndexBuffer::Create(s_Indices, sizeof(s_Indices) / sizeof(uint32_t)));

			pig::BufferLayout buffLayout = {
				{ pig::ShaderDataType::Float3, "POSITION" },
				{ pig::ShaderDataType::Float4, "COLOR" }
			};

			m_Shader.reset(pig::Shader::Create(s_VsCode, s_PsCode, buffLayout));
		}

		~ExampleLayer()
		{
			m_Shader.reset();

			m_VertexBuffer.reset();
			m_IndexBuffer.reset();
		}

		void OnUpdate(pig::Timestep ts) override
		{
			pig::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.f });

			if (pig::Input::IsKeyPressed(PG_KEY_LEFT))
				m_CameraPosition.x -= m_CameraMoveSpeed * ts;
			else if (pig::Input::IsKeyPressed(PG_KEY_RIGHT))
				m_CameraPosition.x += m_CameraMoveSpeed * ts;

			if (pig::Input::IsKeyPressed(PG_KEY_UP))
				m_CameraPosition.y += m_CameraMoveSpeed * ts;
			else if (pig::Input::IsKeyPressed(PG_KEY_DOWN))
				m_CameraPosition.y -= m_CameraMoveSpeed * ts;

			if (pig::Input::IsKeyPressed(PG_KEY_A))
				m_CameraRotation += m_CameraRotationSpeed * ts;
			if (pig::Input::IsKeyPressed(PG_KEY_D))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			pig::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			pig::RenderCommand::Clear();

			m_Camera.SetPosition(m_CameraPosition);
			m_Camera.SetRotation(m_CameraRotation);

			pig::Renderer::BeginScene();
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
					pig::Renderer::Submit(3);
				}
			}
			pig::Renderer::EndScene();

			//if (pigeon::Input::IsKeyPressed(PG_KEY_TAB))
			//	PG_TRACE("Tab key is pressed (poll)!");
		}

		virtual void OnImGuiRender() override
		{
			ImGui::Begin("Settings");
			ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
			ImGui::End();
		}

		void OnEvent(pig::Event& event) override
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
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		std::unique_ptr<pig::VertexBuffer> m_VertexBuffer;
		std::unique_ptr<pig::IndexBuffer> m_IndexBuffer;
		std::unique_ptr<pig::Shader> m_Shader;

		pig::OrthographicCamera m_Camera;
		glm::vec3 m_CameraPosition;
		float m_CameraMoveSpeed = 5.0f;

		float m_CameraRotation = 0.0f;
		float m_CameraRotationSpeed = 180.0f;

		glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

		SceneData m_SceneData;
		std::chrono::steady_clock::time_point m_LastFrameTime;
	};
}

class Sandbox : public pig::Application
{
public:
	Sandbox() = default;
	~Sandbox() = default;
};

pig::S_Ptr<pig::Application> pig::CreateApplication()
{
	pig::S_Ptr<pig::Application> sandbox = Sandbox::Create();
	sandbox->PushLayer(new ExampleLayer());
	return sandbox;
}