#include "pch.h"

#include "Renderer2D.h"

pig::Renderer2D::Data pig::Renderer2D::s_Data;

namespace
{
	static const unsigned int VERTEX_STRIDE = pig::Renderer2D::VERTEX_ATRIB_COUNT * pig::Renderer2D::QUAD_VERTEX_COUNT * sizeof(float);
	static const unsigned int INDEX_STRIDE = pig::Renderer2D::QUAD_INDEX_COUNT * sizeof(int);

	static const float s_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT * pig::Renderer2D::QUAD_VERTEX_COUNT] = {
		-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.f,
		-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.f,
		 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.f
	};

	static const uint32_t s_SuareIndices[pig::Renderer2D::QUAD_INDEX_COUNT] = { 0, 1, 2, 2, 3, 0 };

	static const float s_SquareVerticesEmpty[pig::Renderer2D::VERTEX_ATRIB_COUNT * pig::Renderer2D::QUAD_VERTEX_COUNT * pig::Renderer2D::BATCH_MAX_COUNT];
	static const uint32_t s_SuareIndicesEmpty[pig::Renderer2D::QUAD_INDEX_COUNT * pig::Renderer2D::BATCH_MAX_COUNT];

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

		float m_Data[pig::Renderer2D::VERTEX_ATRIB_COUNT];
	};
	struct QuadData
	{
		QuadData(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& color, unsigned int offsetIndices, int textureId)
		{
			memcpy(m_SquareVertices, VertexData(pos, scale, color, &s_SquareVertices[0], textureId).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT], VertexData(pos, scale, color, &s_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT], textureId).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*2], VertexData(pos, scale, color, &s_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*2], textureId).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*3], VertexData(pos, scale, color, &s_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*3], textureId).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));

			m_SquareIndices[0] = s_SuareIndices[0] + offsetIndices;
			m_SquareIndices[1] = s_SuareIndices[1] + offsetIndices;
			m_SquareIndices[2] = s_SuareIndices[2] + offsetIndices;
			m_SquareIndices[3] = s_SuareIndices[3] + offsetIndices;
			m_SquareIndices[4] = s_SuareIndices[4] + offsetIndices;
			m_SquareIndices[5] = s_SuareIndices[5] + offsetIndices;
		}
		~QuadData() = default;

		float m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*4];
		uint32_t m_SquareIndices[pig::Renderer2D::QUAD_INDEX_COUNT];
	};
}

void pig::Renderer2D::Init()
{
	s_Data.m_VertexBuffer = std::move(pig::VertexBuffer::Create(s_SquareVerticesEmpty, BATCH_MAX_COUNT * VERTEX_STRIDE, sizeof(float) * 10));
	s_Data.m_IndexBuffer = std::move(pig::IndexBuffer::Create(s_SuareIndicesEmpty, (BATCH_MAX_COUNT * INDEX_STRIDE) / sizeof(uint32_t)));

	std::vector<unsigned char> data(2 * 2 * 4, 255);
	s_Data.m_TextureMap[""] = std::move(pig::Texture2D::Create(2, 2, 4, data.data()));
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
	s_Data.m_TextureMap[""]->Bind(0);
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

void pig::Renderer2D::AddTexture(const std::string& path, const std::string& handle)
{
	if (!handle.empty())
	{
		s_Data.m_TextureMap[handle] = std::move(pig::Texture2D::Create(path));
	}
	else
	{
		PG_CORE_ASSERT(false, "Tried to add an empty texture to the 2d batch renderer");
	}
}

void pig::Renderer2D::AddTexture(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data, const std::string& handle)
{
	if (!handle.empty())
	{
		s_Data.m_TextureMap[handle] = std::move(pig::Texture2D::Create(width, height, channels, data));
	}
	else
	{
		PG_CORE_ASSERT(false, "Tried to add an empty texture to the 2d batch renderer");
	}
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col)
{
	if (s_Data.m_TextureMap.find("") != s_Data.m_TextureMap.end())
	{
		pig::Renderer2D::Data::BatchData& texBatch = s_Data.m_BatchMap[""];

		if (texBatch.m_IndexCount == BATCH_MAX_COUNT * QUAD_INDEX_COUNT)
		{
			Flush();
			DrawQuad(pos, scale, col);
		}
		else
		{
			QuadData quad(pos, scale, glm::vec4(col, 1.f), texBatch.m_VertexCount, 0);
			const unsigned int vertexBufferOffset = texBatch.m_VertexCount * VERTEX_ATRIB_COUNT;
			const unsigned int indexBufferOffset = texBatch.m_IndexCount;

			memcpy(&texBatch.m_VertexBuffer[vertexBufferOffset], quad.m_SquareVertices, VERTEX_STRIDE);
			memcpy(&texBatch.m_IndexBuffer[indexBufferOffset], quad.m_SquareIndices, INDEX_STRIDE);

			texBatch.m_IndexCount += 6;
			texBatch.m_VertexCount += 4;
		}
	}
	else
	{
		PG_CORE_ASSERT(false, "White texture not fount in renderer2d batch map");
	}
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const std::string& handle)
{
	if (s_Data.m_TextureMap.find(handle) != s_Data.m_TextureMap.end())
	{
		pig::Renderer2D::Data::BatchData& texBatch = s_Data.m_BatchMap[handle];

		if (texBatch.m_IndexCount == BATCH_MAX_COUNT * QUAD_INDEX_COUNT)
		{
			Flush();
			DrawQuad(pos, scale, handle);
		}
		else
		{
			QuadData quad(pos, scale, glm::vec4(1.f), texBatch.m_VertexCount, 0);
			const unsigned int vertexBufferOffset = texBatch.m_VertexCount * VERTEX_ATRIB_COUNT;
			const unsigned int indexBufferOffset = texBatch.m_IndexCount;

			memcpy(&texBatch.m_VertexBuffer[vertexBufferOffset], quad.m_SquareVertices, VERTEX_STRIDE);
			memcpy(&texBatch.m_IndexBuffer[indexBufferOffset], quad.m_SquareIndices, INDEX_STRIDE);

			texBatch.m_IndexCount += 6;
			texBatch.m_VertexCount += 4;
		}
	}
	else
	{
		PG_CORE_ASSERT(false, "Texture %s not fount in renderer2d batch map", handle.c_str());
	}
}

void pig::Renderer2D::Flush()
{
	for (auto& batch : s_Data.m_BatchMap)
	{
		auto& tex = s_Data.m_TextureMap.find(batch.first);
		PG_CORE_ASSERT(tex != s_Data.m_TextureMap.end(), "unable to bind texture, texture not found");
		if (tex != s_Data.m_TextureMap.end())
		{
			tex->second->Bind(0);
		}
		s_Data.m_VertexBuffer->SetVertices(batch.second.m_VertexBuffer, batch.second.m_VertexCount, 0);
		s_Data.m_IndexBuffer->SetIndices(batch.second.m_IndexBuffer, batch.second.m_IndexCount, 0);
		pig::Renderer2D::Submit(batch.second.m_IndexCount);
	}
	s_Data.m_BatchMap.clear();
}

void pig::Renderer2D::Submit(unsigned int count)
{
	pig::RenderCommand::DrawIndexed(count);
}

void pig::Renderer2D::Destroy()
{
	s_Data.m_VertexBuffer.reset();
	s_Data.m_IndexBuffer.reset();
	s_Data.m_Shader.reset();
	s_Data.m_BatchMap.clear();

	s_Data.m_TextureMap.clear();
}