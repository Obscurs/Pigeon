#include <Pigeon.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"

#include "Pigeon/Renderer/OrthographicCamera.h"
#include "Pigeon/Renderer/Texture.h"
namespace
{
	float s_SquareVertices[5 * 4] = {
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
	};

	uint32_t s_SuareIndices[6] = { 0, 1, 2, 2, 3, 0 };

	class ExampleLayer : public pig::Layer
	{
	public:
		ExampleLayer()
			: Layer("Example"),
			m_Camera(-1.6f, 1.6f, -0.9f, 0.9f),
			m_CameraPosition(0.0f)
		{
			m_VertexBuffer = std::move(pig::VertexBuffer::Create(s_SquareVertices, sizeof(s_SquareVertices), sizeof(float) * 5));
			m_IndexBuffer = std::move(pig::IndexBuffer::Create(s_SuareIndices, sizeof(s_SuareIndices) / sizeof(uint32_t)));

			pig::BufferLayout buffLayout = {
				{ pig::ShaderDataType::Float3, "POSITION" },
				{ pig::ShaderDataType::Float2, "TEXCOORD" }
			};

			m_Shader = std::move(pig::Shader::Create("Assets/Shaders/TestShader.shader", buffLayout));
			m_ShaderTexture = std::move(pig::Shader::Create("Assets/Shaders/TextureShader.shader", buffLayout));

			m_Texture = pig::Texture2D::Create("Assets/Textures/Checkerboard.png");
			m_TextureAlpha = pig::Texture2D::Create("Assets/Textures/alphaTest.png");
		}

		~ExampleLayer()
		{
			m_Shader.reset();
			m_ShaderTexture.reset();

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

			//DRAW GRID
			{
				m_Shader->Bind();
				m_Shader->UploadUniformMat4("u_ViewProjection", m_SceneData.ViewProjectionMatrix);
				m_Shader->UploadUniformFloat3("u_Color", m_SquareColor);

				glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.07f));
				for (int y = -10; y < 10; y++)
				{
					for (int x = -10; x < 10; x++)
					{
						glm::vec3 pos(x * 0.11f, y * 0.11f, 0.f);
						glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
						m_Shader->UploadUniformMat4("u_Transform", transform);
						pig::Renderer::Submit(6);
					}
				}
			}
			//DRAW TEXTURE
			{
				m_ShaderTexture->Bind();
				m_ShaderTexture->UploadUniformMat4("u_ViewProjection", m_SceneData.ViewProjectionMatrix);
				glm::vec3 pos(0.f, 0.f, 0.f);
				glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(1.f));
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				m_ShaderTexture->UploadUniformMat4("u_Transform", transform);
				m_Texture->Bind(0);
				pig::Renderer::Submit(6);

				m_TextureAlpha->Bind(0);
				pig::Renderer::Submit(6);
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

		std::unique_ptr<pig::VertexBuffer> m_VertexBuffer = nullptr;
		std::unique_ptr<pig::IndexBuffer> m_IndexBuffer = nullptr;
		std::unique_ptr<pig::Shader> m_Shader = nullptr;

		std::unique_ptr<pig::Shader> m_ShaderTexture = nullptr;
		pig::U_Ptr<pig::Texture2D> m_Texture;
		pig::U_Ptr<pig::Texture2D> m_TextureAlpha;

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
	sandbox->PushLayer(std::make_shared<ExampleLayer>());
	return sandbox;
}