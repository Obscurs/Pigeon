#include "Sandbox2D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"

#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/RenderCommand.h"

namespace
{
	float s_SquareVertices[5 * 4] = {
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
	};

	uint32_t s_SuareIndices[6] = { 0, 1, 2, 2, 3, 0 };
}

sbx::Sandbox2D::Sandbox2D(): 
	pig::Layer("Sandbox2D"),
	m_CameraController(1280.0f / 720.0f)
{
	m_VertexBuffer = std::move(pig::VertexBuffer::Create(s_SquareVertices, sizeof(s_SquareVertices), sizeof(float) * 5));
	m_IndexBuffer = std::move(pig::IndexBuffer::Create(s_SuareIndices, sizeof(s_SuareIndices) / sizeof(uint32_t)));

	//ARNAU TODO read from shader file
	pig::BufferLayout buffLayout = {
		{ pig::ShaderDataType::Float3, "POSITION" },
		{ pig::ShaderDataType::Float2, "TEXCOORD" }
	};

	//ARNAU TODO SINGLE SHADER
	m_Shader = std::move(pig::Shader::Create("Assets/Shaders/FlatColQuad.shader", buffLayout));

	//ARNAU TODO? Spritesheet
	//m_Texture = pig::Texture2D::Create("Assets/Textures/Checkerboard.png");
}

sbx::Sandbox2D::~Sandbox2D()
{
	m_Shader.reset();

	m_VertexBuffer.reset();
	m_IndexBuffer.reset();
}

void sbx::Sandbox2D::OnUpdate(pig::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	pig::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	pig::RenderCommand::Clear();

	pig::Renderer::BeginScene();

	m_SceneData.ViewProjectionMatrix = m_CameraController.GetCamera().GetViewProjectionMatrix();

	m_VertexBuffer->Bind();
	m_IndexBuffer->Bind();

	m_Shader->Bind();
	m_Shader->UploadUniformMat4("u_ViewProjection", m_SceneData.ViewProjectionMatrix);
	m_Shader->UploadUniformFloat3("u_Color", m_SquareColor);

	m_Shader->UploadUniformMat4("u_ViewProjection", m_SceneData.ViewProjectionMatrix);
	glm::vec3 pos(0.f, 0.f, 0.f);
	glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(1.f));
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
	m_Shader->UploadUniformMat4("u_Transform", transform);
	pig::Renderer::Submit(6);

	pig::Renderer::EndScene();
}

void sbx::Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void sbx::Sandbox2D::OnEvent(pig::Event& event)
{
	m_CameraController.OnEvent(event);
}
