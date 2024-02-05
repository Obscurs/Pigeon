#include "pch.h"

#include "Renderer2D.h"

pig::Renderer2D::Data pig::Renderer2D::s_Data;

namespace
{
	static const float s_SquareVertices[5 * 4] = {
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
	};

	static const uint32_t s_SuareIndices[6] = { 0, 1, 2, 2, 3, 0 };
}

void pig::Renderer2D::Init()
{
	s_Data.m_VertexBuffer = std::move(pig::VertexBuffer::Create(s_SquareVertices, sizeof(s_SquareVertices), sizeof(float) * 5));
	s_Data.m_IndexBuffer = std::move(pig::IndexBuffer::Create(s_SuareIndices, sizeof(s_SuareIndices) / sizeof(uint32_t)));

	std::vector<unsigned char> data(2 * 2 * 4, 255);
	s_Data.m_WhiteTexture = pig::Texture2D::Create(2, 2, 4, data.data());

	s_Data.m_Shader = std::move(pig::Shader::Create("Assets/Shaders/Renderer2DShader.shader"));
}

void pig::Renderer2D::Clear(const glm::vec4& color)
{
	pig::RenderCommand::SetClearColor(color);
	pig::RenderCommand::Clear();
}

void pig::Renderer2D::BeginScene(const pig::OrthographicCameraController& cameraController)
{
	pig::RenderCommand::Begin();

	s_Data.m_Camera = &cameraController;

	s_Data.m_VertexBuffer->Bind();
	s_Data.m_IndexBuffer->Bind();
	s_Data.m_WhiteTexture->Bind(0);
	s_Data.m_Shader->Bind();

	const OrthographicCamera& ortoCamera = s_Data.m_Camera->GetCamera();
	glm::mat4 viewProjMat = ortoCamera.GetViewProjectionMatrix();
	s_Data.m_Shader->UploadUniformMat4("u_ViewProjection", viewProjMat);
}

void pig::Renderer2D::EndScene()
{
	pig::RenderCommand::End();
	s_Data.m_Camera = nullptr;
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col)
{
	glm::mat4 scaleMat = glm::scale(glm::mat4(1.f), scale);
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scaleMat;

	s_Data.m_Shader->UploadUniformMat4("u_Transform", transform);
	s_Data.m_Shader->UploadUniformFloat4("u_Color", glm::vec4(col,1.f));

	pig::Renderer2D::Submit(6);
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const pig::Texture2D& texture)
{
	glm::mat4 scaleMat = glm::scale(glm::mat4(1.f), scale);
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scaleMat;

	s_Data.m_Shader->UploadUniformMat4("u_Transform", transform);
	s_Data.m_Shader->UploadUniformFloat4("u_Color", glm::vec4(1.f));
	texture.Bind(0);

	pig::Renderer2D::Submit(6);
}

void pig::Renderer2D::Submit(unsigned int count)
{
	pig::RenderCommand::DrawIndexed(count);
}
