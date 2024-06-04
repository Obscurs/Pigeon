#include "pch.h"

#include "Renderer2D.h"

#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Msdfdata.h"
#include "Pigeon/Renderer/Texture.h"

pig::Renderer2D::Data pig::Renderer2D::s_Data;
const pig::UUID pig::Renderer2D::s_DefaultTexture = pig::UUID::Generate();

namespace
{
	static const unsigned int VERTEX_STRIDE = pig::Renderer2D::VERTEX_ATRIB_COUNT * pig::Renderer2D::QUAD_VERTEX_COUNT * sizeof(float);
	static const unsigned int INDEX_STRIDE = pig::Renderer2D::QUAD_INDEX_COUNT * sizeof(int);

	static const uint32_t s_SuareIndices[pig::Renderer2D::QUAD_INDEX_COUNT] = { 0, 2, 1, 2, 0, 3 };

	static const float s_SquareVerticesEmpty[pig::Renderer2D::VERTEX_ATRIB_COUNT * pig::Renderer2D::QUAD_VERTEX_COUNT * pig::Renderer2D::BATCH_MAX_COUNT];
	static const uint32_t s_SuareIndicesEmpty[pig::Renderer2D::QUAD_INDEX_COUNT * pig::Renderer2D::BATCH_MAX_COUNT];

	struct VertexData
	{
		VertexData(const glm::vec4& pos, const glm::vec4& color, int textureId, const glm::vec2& texCoords)
		{
			m_Data[pig::Renderer2D::ATRIB_POS_X_INDEX] = pos.x;
			m_Data[pig::Renderer2D::ATRIB_POS_Y_INDEX] = pos.y * (pig::Texture2D::FlipY() ? -1.f : 1.f);
			m_Data[pig::Renderer2D::ATRIB_POS_Z_INDEX] = pos.z;
			m_Data[pig::Renderer2D::ATRIB_COL_R_INDEX] = color.r;
			m_Data[pig::Renderer2D::ATRIB_COL_G_INDEX] = color.g;
			m_Data[pig::Renderer2D::ATRIB_COL_B_INDEX] = color.b;
			m_Data[pig::Renderer2D::ATRIB_COL_A_INDEX] = color.a;
			m_Data[pig::Renderer2D::ATRIB_TEX_X_INDEX] = texCoords.x;
			m_Data[pig::Renderer2D::ATRIB_TEX_Y_INDEX] = texCoords.y;
			m_Data[pig::Renderer2D::ATRIB_TEX_ID_INDEX] = textureId;
		}

		~VertexData() = default;

		float m_Data[pig::Renderer2D::VERTEX_ATRIB_COUNT];
	};
	struct QuadData
	{
		QuadData(const glm::mat4& transform, const glm::vec4& color, unsigned int offsetIndices, int textureId, const glm::vec4& texCoordsRect, const glm::vec3& origin)
		{
			const glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -origin);
			const glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), origin);
			const glm::mat4 combinedTransform = translateBack * transform * translateToOrigin;

			const glm::vec4 pos_v1 = combinedTransform * glm::vec4(0, 0, 0, 1);
			const glm::vec4 pos_v2 = combinedTransform * glm::vec4(0, 1, 0, 1);
			const glm::vec4 pos_v3 = combinedTransform * glm::vec4(1, 1, 0, 1);
			const glm::vec4 pos_v4 = combinedTransform * glm::vec4(1, 0, 0, 1);

			memcpy(m_SquareVertices, VertexData(pos_v1, color, textureId, glm::vec2(texCoordsRect.x, texCoordsRect.y)).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT], VertexData(pos_v2, color, textureId, glm::vec2(texCoordsRect.x, texCoordsRect.w)).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*2], VertexData(pos_v3, color, textureId, glm::vec2(texCoordsRect.z, texCoordsRect.w)).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::Renderer2D::VERTEX_ATRIB_COUNT*3], VertexData(pos_v4, color, textureId, glm::vec2(texCoordsRect.z, texCoordsRect.y)).m_Data, pig::Renderer2D::VERTEX_ATRIB_COUNT * sizeof(float));

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

	pig::MappedTexture mappedTexture = { std::move(pig::Texture2D::Create(2, 2, 4, data.data())), pig::EMappedTextureType::eQuad };
	s_Data.m_TextureMap[pig::Renderer2D::s_DefaultTexture] = std::move(mappedTexture);
	s_Data.m_QuadShader = std::move(pig::Shader::Create("Assets/Shaders/Renderer2DQuad.shader"));
	s_Data.m_TextShader = std::move(pig::Shader::Create("Assets/Shaders/Renderer2DText.shader"));
}

void pig::Renderer2D::Clear(const glm::vec4& color)
{
	pig::RenderCommand::SetClearColor(color);
	pig::RenderCommand::Clear();
}

void pig::Renderer2D::BeginScene(const pig::OrthographicCamera& ortoCamera)
{
	pig::RenderCommand::Begin();

	s_Data.m_VertexBuffer->Bind();
	s_Data.m_IndexBuffer->Bind();
	s_Data.m_TextureMap[pig::Renderer2D::s_DefaultTexture].m_Texture->Bind(0);
	s_Data.m_QuadShader->Bind();

	glm::mat4 viewProjMat = ortoCamera.GetViewProjectionMatrix();
	s_Data.m_QuadShader->UploadUniformMat4("u_ViewProjection", viewProjMat);
}

void pig::Renderer2D::EndScene()
{
	Flush();

	pig::RenderCommand::End();
	s_Data.m_VertexBuffer->Unbind();
	s_Data.m_IndexBuffer->Unbind();
}

const pig::Texture2D& pig::Renderer2D::GetTexture(const pig::UUID& textureID)
{
	const auto it = s_Data.m_TextureMap.find(textureID);
	if (it != s_Data.m_TextureMap.end())
	{
		return *it->second.m_Texture.get();
	}
	else
	{
		PG_CORE_ASSERT(false, "Texture not found returning default one");

		return *s_Data.m_TextureMap[pig::Renderer2D::s_DefaultTexture].m_Texture.get();
	}
}

pig::UUID pig::Renderer2D::AddTexture(const std::string& path, EMappedTextureType type)
{
	pig::UUID textureID = pig::UUID::Generate();
	pig::MappedTexture mappedTexture = { std::move(pig::Texture2D::Create(path)), type };
	s_Data.m_TextureMap[textureID] = std::move(mappedTexture);
	return textureID;
}

pig::UUID pig::Renderer2D::AddTexture(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data, EMappedTextureType type)
{
	pig::UUID textureID = pig::UUID::Generate();
	pig::MappedTexture mappedTexture = { std::move(pig::Texture2D::Create(width, height, channels, data)), type };
	s_Data.m_TextureMap[textureID] = std::move(mappedTexture);
	return textureID;
}

void pig::Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec3& col, const glm::vec3& origin)
{
	DrawBatch(transform, col, pig::Renderer2D::s_DefaultTexture, glm::vec4(0.f, 0.f, 1.f, 1.f), origin);
}

void pig::Renderer2D::DrawSprite(const pig::Sprite& sprite)
{
	DrawBatch(sprite.GetTransform(), glm::vec3(1.f), sprite.GetTextureID(), sprite.GetTexCoordsRect(), sprite.GetOrigin());
}

void pig::Renderer2D::DrawString(const glm::mat4& transform, const std::string& string, pig::S_Ptr<pig::Font> font, const glm::vec4& color, float kerning, float linespacing)
{
	const pig::Texture2D& fontAtlas = GetTexture(font->GetFontID());

	glm::dvec2 charOffset{ 0.0, 0.0 };
	const glm::vec3 originSprite(0.f, 0.0f, 0.f);

	for (size_t i = 0; i < string.size(); i++)
	{
		char character = string[i];

		if (font->IsCharacterDrawable(character))
		{
			const glm::vec4 texCoords = font->GetCharacterTexCoordsQuad(character, fontAtlas);
			const glm::vec4 charQuad = font->GetCharacterVertexQuad(character, charOffset);
			const glm::mat4 charTransform = font->GetCharacterTransform(charQuad, transform);

			DrawBatch(charTransform, color, font->GetFontID(), texCoords, originSprite);
		}

		if (font->IsCharacterNewLine(character))
		{
			charOffset.x = 0.0;
		}
		if (i < string.size() - 1)
		{
			glm::dvec2 charAdvance = font->GetCharacterAdvance(character, string[i + 1], kerning, linespacing);
			charOffset += charAdvance;
		}
	}
}

void pig::Renderer2D::DrawQuad(const glm::mat4& transform, const pig::UUID& textureID, const glm::vec3& origin)
{
	//TODO ARNAU Assert if scene has not began
	DrawBatch(transform, glm::vec3(1.f), textureID, glm::vec4(0.f, 0.f, 1.f, 1.f), origin);
}

void pig::Renderer2D::DrawBatch(const glm::mat4& transform, const glm::vec3& col, const pig::UUID& textureID, glm::vec4 texRect, const glm::vec3& origin)
{
	if (s_Data.m_TextureMap.find(textureID) != s_Data.m_TextureMap.end())
	{
		pig::Renderer2D::Data::BatchData& texBatch = s_Data.m_BatchMap[textureID];

		if (texBatch.m_IndexCount == BATCH_MAX_COUNT * QUAD_INDEX_COUNT)
		{
			Flush();
			DrawBatch(transform, col, textureID, texRect, origin);
		}
		else
		{
			QuadData quad(transform, glm::vec4(col, 1.f), texBatch.m_VertexCount, 0, texRect, origin);
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
		PG_CORE_ASSERT(false, "Texture %s not fount in renderer2d batch map", textureID.ToString().c_str());
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
			tex->second.m_Texture->Bind(0);
			switch (tex->second.m_TextureType)
			{
			case pig::EMappedTextureType::eQuad:
				s_Data.m_QuadShader->Bind(); break;
			case pig::EMappedTextureType::eText:
				s_Data.m_TextShader->Bind(); break;
			default:
				PG_CORE_ASSERT(false, "EMappedTextureType not implemented");
			}
			s_Data.m_VertexBuffer->SetVertices(batch.second.m_VertexBuffer, batch.second.m_VertexCount, 0);
			s_Data.m_IndexBuffer->SetIndices(batch.second.m_IndexBuffer, batch.second.m_IndexCount, 0);
			pig::Renderer2D::Submit(batch.second.m_IndexCount);
		}
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