#include "pch.h"

#include "Renderer2D.h"

#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Msdfdata.h"

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

	pig::MappedTexture mappedTexture = { std::move(pig::Texture2D::Create(2, 2, 4, data.data())), pig::EMappedTextureType::eQuad };
	s_Data.m_TextureMap[""] = std::move(mappedTexture);
	s_Data.m_QuadShader = std::move(pig::Shader::Create("Assets/Shaders/Renderer2DQuad.shader"));
	s_Data.m_TextShader = std::move(pig::Shader::Create("Assets/Shaders/Renderer2DText.shader"));
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
	s_Data.m_TextureMap[""].m_Texture->Bind(0);
	s_Data.m_QuadShader->Bind();

	const OrthographicCamera& ortoCamera = s_Data.m_Camera->GetCamera();
	glm::mat4 viewProjMat = ortoCamera.GetViewProjectionMatrix();
	s_Data.m_QuadShader->UploadUniformMat4("u_ViewProjection", viewProjMat);
}

void pig::Renderer2D::EndScene()
{
	Flush();

	pig::RenderCommand::End();
	s_Data.m_VertexBuffer->Unbind();
	s_Data.m_IndexBuffer->Unbind();
	s_Data.m_Camera = nullptr;
}

const pig::S_Ptr<pig::Texture2D> pig::Renderer2D::GetTexture(const std::string& handle)
{
	const auto it = s_Data.m_TextureMap.find(handle);
	if (it != s_Data.m_TextureMap.end())
	{
		return it->second.m_Texture;
	}
	else
	{
		//TODO Arnau: ensure default texture exists as well, maybe create some kind of null texture to return in this cases
		PG_CORE_ASSERT(false, "Texture not found returning default one");
		return s_Data.m_TextureMap[""].m_Texture;
	}
}

void pig::Renderer2D::AddTexture(const std::string& path, const std::string& handle, EMappedTextureType type)
{
	if (!handle.empty())
	{
		pig::MappedTexture mappedTexture = { std::move(pig::Texture2D::Create(path)), type };
		s_Data.m_TextureMap[handle] = std::move(mappedTexture);
	}
	else
	{
		PG_CORE_ASSERT(false, "Tried to add an empty texture to the 2d batch renderer");
	}
}

void pig::Renderer2D::AddTexture(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data, const std::string& handle, EMappedTextureType type)
{
	if (!handle.empty())
	{
		pig::MappedTexture mappedTexture = { std::move(pig::Texture2D::Create(width, height, channels, data)), type };
		s_Data.m_TextureMap[handle] = std::move(mappedTexture);
	}
	else
	{
		PG_CORE_ASSERT(false, "Tried to add an empty texture to the 2d batch renderer");
	}
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col)
{
	DrawBatch(pos, scale, col, "", glm::vec4(0.f, 0.f, 1.f, 1.f));
}

void pig::Renderer2D::DrawSprite(const pig::Sprite& sprite)
{
	DrawBatch(sprite.GetPosition(), glm::vec3(sprite.GetScale(), 1.0f), glm::vec3(1.f), sprite.GetTextureID(), sprite.GetTexCoordsRect());
}

void pig::Renderer2D::DrawString(const std::string& string, pig::S_Ptr<pig::Font> font, const glm::mat4& transform, const glm::vec4& color, float kerning, float linespacing)
{
	const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
	const auto& metrics = fontGeometry.getMetrics();
	pig::S_Ptr<Texture2D> fontAtlas = GetTexture(font->GetFontID());

	double x = 0.0;
	double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
	double y = 0.0;

	const float spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();

	for (size_t i = 0; i < string.size(); i++)
	{
		char character = string[i];
		if (character == '\r')
			continue;

		if (character == '\n')
		{
			x = 0;
			y -= fsScale * metrics.lineHeight + linespacing;
			continue;
		}

		if (character == ' ')
		{
			float advance = spaceGlyphAdvance;
			if (i < string.size() - 1)
			{
				char nextCharacter = string[i + 1];
				double dAdvance;
				fontGeometry.getAdvance(dAdvance, character, nextCharacter);
				advance = (float)dAdvance;
			}

			x += fsScale * advance + kerning;
			continue;
		}

		if (character == '\t')
		{
			x += 4.0f * (fsScale * spaceGlyphAdvance + kerning);
			continue;
		}

		auto glyph = fontGeometry.getGlyph(character);
		if (!glyph)
			glyph = fontGeometry.getGlyph('?');
		if (!glyph)
			return;

		double al, ab, ar, at;
		glyph->getQuadAtlasBounds(al, ab, ar, at);
		glm::vec2 texCoordMin((float)al, (float)ab);
		glm::vec2 texCoordMax((float)ar, (float)at);

		double pl, pb, pr, pt;
		glyph->getQuadPlaneBounds(pl, pb, pr, pt);
		glm::vec2 quadMin((float)pl, (float)pb);
		glm::vec2 quadMax((float)pr, (float)pt);

		quadMin *= fsScale, quadMax *= fsScale;
		quadMin += glm::vec2(x, y);
		quadMax += glm::vec2(x, y);

		float texelWidth = 1.0f / fontAtlas->GetWidth();
		float texelHeight = 1.0f / fontAtlas->GetHeight();
		texCoordMin *= glm::vec2(texelWidth, texelHeight);
		texCoordMax *= glm::vec2(texelWidth, texelHeight);

		//TODO Arnau: transform charater to the proper position instead of mid
		const glm::vec2 sizequad((quadMax.x - quadMin.x), (quadMax.y - quadMin.y));
		const glm::vec2 midPoint(sizequad.x / 2.f + quadMin.x, sizequad.y / 2.f + quadMin.y);
		glm::vec3 position = transform * glm::vec4(midPoint, 0.0f, 1.0f);
		DrawBatch(position, glm::vec3(sizequad, 1.0f), color, font->GetFontID(), glm::vec4(texCoordMin, texCoordMax));

		if (i < string.size() - 1)
		{
			double advance = glyph->getAdvance();
			char nextCharacter = string[i + 1];
			fontGeometry.getAdvance(advance, character, nextCharacter);

			x += fsScale * advance + kerning;
		}
	}
}

void pig::Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const std::string& handle)
{
	DrawBatch(pos, scale, glm::vec3(1.f), handle, glm::vec4(0.f, 0.f, 1.f, 1.f));
}

void pig::Renderer2D::DrawBatch(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col, const std::string& handle, glm::vec4 texRect)
{
	if (s_Data.m_TextureMap.find(handle) != s_Data.m_TextureMap.end())
	{
		pig::Renderer2D::Data::BatchData& texBatch = s_Data.m_BatchMap[handle];

		if (texBatch.m_IndexCount == BATCH_MAX_COUNT * QUAD_INDEX_COUNT)
		{
			Flush();
			DrawBatch(pos, scale, col, handle, texRect);
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