#include "pch.h"

#include "Renderer2D.h"

pig::Renderer2D::Data pig::Renderer2D::s_Data;

namespace
{
	static const unsigned int BATCH_MAX_COUNT = 1000;

	static const float s_SquareVertices[10 * 4] = {
		-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.f,
		-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.f,
		 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.f
	};

	static const uint32_t s_SuareIndices[6] = { 0, 1, 2, 2, 3, 0 };

	static const float s_SquareVerticesEmpty[10 * BATCH_MAX_COUNT];
	static const uint32_t s_SuareIndicesEmpty[6 * BATCH_MAX_COUNT];

	struct VertexData
	{
		VertexData(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& color, const float* vertexBase, int textureId)
		{
			m_Data[0] = vertexBase[0] * scale.x + pos.x;
			m_Data[1] = vertexBase[1] * scale.y + pos.y;
			m_Data[2] = vertexBase[2] * scale.z + pos.z;
			m_Data[3] = color.r;
			m_Data[4] = color.g;
			m_Data[5] = color.b;
			m_Data[6] = color.a;
			m_Data[7] = vertexBase[7];
			m_Data[8] = vertexBase[8];
			m_Data[9] = textureId;
		}

		~VertexData() = default;

		float m_Data[10];
	};
	struct QuadData
	{
		QuadData(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& color, unsigned int offsetIndices, int textureId)
		{
			memcpy(m_SquareVertices, VertexData(pos, scale, color, &s_SquareVertices[0], textureId).m_Data, 10 * sizeof(float));
			memcpy(&m_SquareVertices[10], VertexData(pos, scale, color, &s_SquareVertices[10], textureId).m_Data, 10 * sizeof(float));
			memcpy(&m_SquareVertices[20], VertexData(pos, scale, color, &s_SquareVertices[20], textureId).m_Data, 10 * sizeof(float));
			memcpy(&m_SquareVertices[30], VertexData(pos, scale, color, &s_SquareVertices[30], textureId).m_Data, 10 * sizeof(float));

			m_SquareIndices[0] = s_SuareIndices[0] + offsetIndices;
			m_SquareIndices[1] = s_SuareIndices[1] + offsetIndices;
			m_SquareIndices[2] = s_SuareIndices[2] + offsetIndices;
			m_SquareIndices[3] = s_SuareIndices[3] + offsetIndices;
			m_SquareIndices[4] = s_SuareIndices[4] + offsetIndices;
			m_SquareIndices[5] = s_SuareIndices[5] + offsetIndices;
		}
		~QuadData() = default;

		float m_SquareVertices[40];
		uint32_t m_SquareIndices[6];
	};
}

void pig::Renderer2D::Init()
{
	s_Data.m_VertexBuffer = std::move(pig::VertexBuffer::Create(s_SquareVerticesEmpty, sizeof(float) * 10 * BATCH_MAX_COUNT, sizeof(float) * 10));
	s_Data.m_IndexBuffer = std::move(pig::IndexBuffer::Create(s_SuareIndicesEmpty, (sizeof(float) * BATCH_MAX_COUNT) / sizeof(uint32_t)));

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
	Flush();

	pig::RenderCommand::End();
	s_Data.m_VertexBuffer->Unbind();
	s_Data.m_IndexBuffer->Unbind();
	s_Data.m_Camera = nullptr;
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col)
{
	QuadData quad(pos, scale, glm::vec4(col, 1.f), s_Data.m_VertexCount, 0);

	s_Data.m_VertexBuffer->AppendVertices(quad.m_SquareVertices, 4, s_Data.m_VertexCount);
	s_Data.m_IndexBuffer->AppendIndices(quad.m_SquareIndices, 6, s_Data.m_IndexCount);

	s_Data.m_IndexCount += 6;
	s_Data.m_VertexCount += 4;
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const pig::Texture2D& texture)
{
	texture.Bind(1);
	
	QuadData quad(pos, scale, glm::vec4(1.f), s_Data.m_VertexCount, 1);
	s_Data.m_VertexBuffer->AppendVertices(quad.m_SquareVertices, 4, s_Data.m_VertexCount);
	s_Data.m_IndexBuffer->AppendIndices(quad.m_SquareIndices, 6, s_Data.m_IndexCount);

	s_Data.m_IndexCount += 6;
	s_Data.m_VertexCount += 4;
}

void pig::Renderer2D::Flush()
{
	pig::Renderer2D::Submit(s_Data.m_IndexCount);
	s_Data.m_VertexCount = 0;
	s_Data.m_IndexCount = 0;
}

void pig::Renderer2D::Submit(unsigned int count)
{
	pig::RenderCommand::DrawIndexed(count);
}

void pig::Renderer2D::Destroy()
{
	s_Data.m_Shader.reset();
	s_Data.m_IndexBuffer.reset();
	s_Data.m_WhiteTexture.reset();
	s_Data.m_VertexBuffer.reset();
	s_Data.m_IndexBuffer.reset();
}