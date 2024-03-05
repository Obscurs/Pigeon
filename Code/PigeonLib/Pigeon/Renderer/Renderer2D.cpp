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
		VertexData(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& color, const float* vertexBase, int textureId, const glm::vec2& texCoords)
		{
			m_Data[0] = vertexBase[0] * scale.x + pos.x;
			m_Data[1] = vertexBase[1] * scale.y + pos.y;
			m_Data[2] = vertexBase[2] * scale.z + pos.z;
			m_Data[3] = color.r;
			m_Data[4] = color.g;
			m_Data[5] = color.b;
			m_Data[6] = color.a;
			m_Data[7] = texCoords.x;
			m_Data[8] = texCoords.y;
			m_Data[9] = textureId;
		}

		~VertexData() = default;

		float m_Data[pig::Renderer2D::VERTEX_ATRIB_COUNT];
	};
	struct QuadData
	{
		QuadData(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& color, unsigned int offsetIndices, int textureId, const glm::vec4& texCoordsRect)
		{
			memcpy(m_SquareVertices, VertexData(pos, scale, color, &s_SquareVertices[0], textureId, glm::vec2(texCoordsRect.x, texCoordsRect.y)).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT], VertexData(pos, scale, color, &s_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT], textureId, glm::vec2(texCoordsRect.x, texCoordsRect.w)).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*2], VertexData(pos, scale, color, &s_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*2], textureId, glm::vec2(texCoordsRect.z, texCoordsRect.w)).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*3], VertexData(pos, scale, color, &s_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*3], textureId, glm::vec2(texCoordsRect.z, texCoordsRect.y)).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));

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

const pig::Texture2D* pig::Renderer2D::GetTexture(const std::string& handle)
{
	const auto it = s_Data.m_TextureMap.find(handle);
	return it != s_Data.m_TextureMap.end() ? it->second.get() : nullptr;
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
	DrawQuad(pos, scale, col, "", glm::vec4(0.f, 0.f, 1.f, 1.f));
}

void pig::Renderer2D::DrawSprite(const pig::Sprite& sprite)
{
	DrawQuad(sprite.GetPosition(), glm::vec3(sprite.GetScale(), 1.0f), glm::vec3(1.f), sprite.GetTextureID(), sprite.GetTexCoordsRect());
}

void pig::Renderer2D::DrawTextSprite(const pig::Sprite& sprite, const std::string& text, const glm::vec3& col)
{
	const pig::Texture2D* texture = pig::Renderer2D::GetTexture(sprite.GetTextureID());
	PG_CORE_ASSERT(texture, "Text texture is missing");
	
	float charWidth = sprite.GetTexCoordsRect().x;
	float charHeight = sprite.GetTexCoordsRect().y;
	int line = 0;
	int charIndex = 0;
	for (unsigned int i = 0; i < text.size(); ++i)
	{
		const char c = text[i];
		if (c == '\n')
		{
			--line;
			charIndex = 0;
			continue;
		}
		glm::vec4 rectangle(0.f, 0.f, 0.f, 0.f);
		if (c == 'A') rectangle = glm::vec4(charWidth * 0, charHeight * 0, charWidth, charHeight);
		else if (c == 'B') rectangle = glm::vec4(charWidth * 1, charHeight * 0, charWidth, charHeight);
		else if (c == 'C') rectangle = glm::vec4(charWidth * 2, charHeight * 0, charWidth, charHeight);
		else if (c == 'D') rectangle = glm::vec4(charWidth * 3, charHeight * 0, charWidth, charHeight);
		else if (c == 'E') rectangle = glm::vec4(charWidth * 4, charHeight * 0, charWidth, charHeight);
		else if (c == 'F') rectangle = glm::vec4(charWidth * 5, charHeight * 0, charWidth, charHeight);
		else if (c == 'G') rectangle = glm::vec4(charWidth * 6, charHeight * 0, charWidth, charHeight);
		else if (c == 'H') rectangle = glm::vec4(charWidth * 7, charHeight * 0, charWidth, charHeight);
		else if (c == 'I') rectangle = glm::vec4(charWidth * 0, charHeight * 1, charWidth, charHeight);
		else if (c == 'J') rectangle = glm::vec4(charWidth * 1, charHeight * 1, charWidth, charHeight);
		else if (c == 'K') rectangle = glm::vec4(charWidth * 2, charHeight * 1, charWidth, charHeight);
		else if (c == 'L') rectangle = glm::vec4(charWidth * 3, charHeight * 1, charWidth, charHeight);
		else if (c == 'M') rectangle = glm::vec4(charWidth * 4, charHeight * 1, charWidth, charHeight);
		else if (c == 'N') rectangle = glm::vec4(charWidth * 5, charHeight * 1, charWidth, charHeight);
		else if (c == 'O') rectangle = glm::vec4(charWidth * 6, charHeight * 1, charWidth, charHeight);
		else if (c == 'P') rectangle = glm::vec4(charWidth * 7, charHeight * 1, charWidth, charHeight);
		else if (c == 'Q') rectangle = glm::vec4(charWidth * 0, charHeight * 2, charWidth, charHeight);
		else if (c == 'R') rectangle = glm::vec4(charWidth * 1, charHeight * 2, charWidth, charHeight);
		else if (c == 'S') rectangle = glm::vec4(charWidth * 2, charHeight * 2, charWidth, charHeight);
		else if (c == 'T') rectangle = glm::vec4(charWidth * 3, charHeight * 2, charWidth, charHeight);
		else if (c == 'U') rectangle = glm::vec4(charWidth * 4, charHeight * 2, charWidth, charHeight);
		else if (c == 'V') rectangle = glm::vec4(charWidth * 5, charHeight * 2, charWidth, charHeight);
		else if (c == 'W') rectangle = glm::vec4(charWidth * 6, charHeight * 2, charWidth, charHeight);
		else if (c == 'X') rectangle = glm::vec4(charWidth * 7, charHeight * 2, charWidth, charHeight);
		else if (c == 'Y') rectangle = glm::vec4(charWidth * 0, charHeight * 3, charWidth, charHeight);
		else if (c == 'Z') rectangle = glm::vec4(charWidth * 1, charHeight * 3, charWidth, charHeight);
		else if (c == 'a') rectangle = glm::vec4(charWidth * 2, charHeight * 3, charWidth, charHeight);
		else if (c == 'b') rectangle = glm::vec4(charWidth * 3, charHeight * 3, charWidth, charHeight);
		else if (c == 'c') rectangle = glm::vec4(charWidth * 4, charHeight * 3, charWidth, charHeight);
		else if (c == 'd') rectangle = glm::vec4(charWidth * 5, charHeight * 3, charWidth, charHeight);
		else if (c == 'e') rectangle = glm::vec4(charWidth * 6, charHeight * 3, charWidth, charHeight);
		else if (c == 'f') rectangle = glm::vec4(charWidth * 7, charHeight * 3, charWidth, charHeight);
		else if (c == 'g') rectangle = glm::vec4(charWidth * 0, charHeight * 4, charWidth, charHeight);
		else if (c == 'h') rectangle = glm::vec4(charWidth * 1, charHeight * 4, charWidth, charHeight);
		else if (c == 'i') rectangle = glm::vec4(charWidth * 2, charHeight * 4, charWidth, charHeight);
		else if (c == 'j') rectangle = glm::vec4(charWidth * 3, charHeight * 4, charWidth, charHeight);
		else if (c == 'k') rectangle = glm::vec4(charWidth * 4, charHeight * 4, charWidth, charHeight);
		else if (c == 'l') rectangle = glm::vec4(charWidth * 5, charHeight * 4, charWidth, charHeight);
		else if (c == 'm') rectangle = glm::vec4(charWidth * 6, charHeight * 4, charWidth, charHeight);
		else if (c == 'n') rectangle = glm::vec4(charWidth * 7, charHeight * 4, charWidth, charHeight);
		else if (c == 'o') rectangle = glm::vec4(charWidth * 0, charHeight * 5, charWidth, charHeight);
		else if (c == 'p') rectangle = glm::vec4(charWidth * 1, charHeight * 5, charWidth, charHeight);
		else if (c == 'q') rectangle = glm::vec4(charWidth * 2, charHeight * 5, charWidth, charHeight);
		else if (c == 'r') rectangle = glm::vec4(charWidth * 3, charHeight * 5, charWidth, charHeight);
		else if (c == 's') rectangle = glm::vec4(charWidth * 4, charHeight * 5, charWidth, charHeight);
		else if (c == 't') rectangle = glm::vec4(charWidth * 5, charHeight * 5, charWidth, charHeight);
		else if (c == 'u') rectangle = glm::vec4(charWidth * 6, charHeight * 5, charWidth, charHeight);
		else if (c == 'v') rectangle = glm::vec4(charWidth * 7, charHeight * 5, charWidth, charHeight);
		else if (c == 'w') rectangle = glm::vec4(charWidth * 0, charHeight * 6, charWidth, charHeight);
		else if (c == 'x') rectangle = glm::vec4(charWidth * 1, charHeight * 6, charWidth, charHeight);
		else if (c == 'y') rectangle = glm::vec4(charWidth * 2, charHeight * 6, charWidth, charHeight);
		else if (c == 'z') rectangle = glm::vec4(charWidth * 3, charHeight * 6, charWidth, charHeight);
		else if (c == '0') rectangle = glm::vec4(charWidth * 4, charHeight * 6, charWidth, charHeight);
		else if (c == '1') rectangle = glm::vec4(charWidth * 5, charHeight * 6, charWidth, charHeight);
		else if (c == '2') rectangle = glm::vec4(charWidth * 6, charHeight * 6, charWidth, charHeight);
		else if (c == '3') rectangle = glm::vec4(charWidth * 7, charHeight * 6, charWidth, charHeight);
		else if (c == '4') rectangle = glm::vec4(charWidth * 0, charHeight * 7, charWidth, charHeight);
		else if (c == '5') rectangle = glm::vec4(charWidth * 1, charHeight * 7, charWidth, charHeight);
		else if (c == '6') rectangle = glm::vec4(charWidth * 2, charHeight * 7, charWidth, charHeight);
		else if (c == '7') rectangle = glm::vec4(charWidth * 3, charHeight * 7, charWidth, charHeight);
		else if (c == '8') rectangle = glm::vec4(charWidth * 4, charHeight * 7, charWidth, charHeight);
		else if (c == '9') rectangle = glm::vec4(charWidth * 5, charHeight * 7, charWidth, charHeight);
		else if (c == '.') rectangle = glm::vec4(charWidth * 6, charHeight * 7, charWidth, charHeight);
		else if (c == ',') rectangle = glm::vec4(charWidth * 7, charHeight * 7, charWidth, charHeight);
		else if (c == ';') rectangle = glm::vec4(charWidth * 0, charHeight * 8, charWidth, charHeight);
		else if (c == ':') rectangle = glm::vec4(charWidth * 1, charHeight * 8, charWidth, charHeight);
		else if (c == '?') rectangle = glm::vec4(charWidth * 2, charHeight * 8, charWidth, charHeight);
		else if (c == '!') rectangle = glm::vec4(charWidth * 3, charHeight * 8, charWidth, charHeight);
		else if (c == '-') rectangle = glm::vec4(charWidth * 4, charHeight * 8, charWidth, charHeight);
		else if (c == '_') rectangle = glm::vec4(charWidth * 5, charHeight * 8, charWidth, charHeight);
		else if (c == '\'') rectangle = glm::vec4(charWidth * 6, charHeight * 8, charWidth, charHeight);
		else if (c == '#') rectangle = glm::vec4(charWidth * 7, charHeight * 8, charWidth, charHeight);
		else if (c == '"') rectangle = glm::vec4(charWidth * 0, charHeight * 9, charWidth, charHeight);
		else if (c == '\\') rectangle = glm::vec4(charWidth * 1, charHeight * 9, charWidth, charHeight);
		else if (c == '/') rectangle = glm::vec4(charWidth * 2, charHeight * 9, charWidth, charHeight);
		else if (c == '<') rectangle = glm::vec4(charWidth * 3, charHeight * 9, charWidth, charHeight);
		else if (c == '>') rectangle = glm::vec4(charWidth * 4, charHeight * 9, charWidth, charHeight);
		else if (c == '(') rectangle = glm::vec4(charWidth * 5, charHeight * 9, charWidth, charHeight);
		else if (c == ')') rectangle = glm::vec4(charWidth * 6, charHeight * 9, charWidth, charHeight);
		else if (c == 'e') rectangle = glm::vec4(charWidth * 7, charHeight * 9, charWidth, charHeight);

		glm::vec3 pos = sprite.GetPosition();
		pos.x += charIndex * (0.5 - sprite.GetTexCoordsRect().z) * sprite.GetScale().x;
		pos.y += line * (0.5 - sprite.GetTexCoordsRect().w) * sprite.GetScale().y;

		const glm::vec4 texCoordsRect = texture->GetTexCoordsRect(glm::vec2(rectangle.x, rectangle.y), glm::vec2(rectangle.z, rectangle.w));
		DrawQuad(pos, glm::vec3(sprite.GetScale(), 1.0f), col, sprite.GetTextureID(), texCoordsRect);
		++charIndex;
	}
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const std::string& handle)
{
	DrawQuad(pos, scale, glm::vec3(1.f), handle, glm::vec4(0.f, 0.f, 1.f, 1.f));
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col, const std::string& handle, glm::vec4 texRect)
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
			QuadData quad(pos, scale, glm::vec4(col, 1.f), texBatch.m_VertexCount, 0, texRect);
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
	s_Data.m_BatchMap.clear();
	s_Data.m_TextureMap.clear();
}