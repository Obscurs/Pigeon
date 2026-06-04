#include "pch.h"
#include "Renderer2DSystem.h"

#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/OrthographicCameraComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Pigeon/Renderer/DrawStringInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIStringInFrameEvent.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Msdfdata.h"
#include "Pigeon/Renderer/RenderCommand.h"
#include "Pigeon/Renderer/RendererDataSingletonComponent.h"
#include "Pigeon/Renderer/Texture.h"

namespace
{
	static const unsigned int VERTEX_STRIDE = pg::VERTEX_ATRIB_COUNT * pg::QUAD_VERTEX_COUNT * sizeof(float);
	static const unsigned int INDEX_STRIDE = pg::QUAD_INDEX_COUNT * sizeof(int);

	static const uint32_t s_SuareIndices[pg::QUAD_INDEX_COUNT] = { 0, 2, 1, 2, 0, 3 };

	static const float s_SquareVerticesEmpty[pg::VERTEX_ATRIB_COUNT * pg::QUAD_VERTEX_COUNT * pg::BATCH_MAX_COUNT];
	static const uint32_t s_SuareIndicesEmpty[pg::QUAD_INDEX_COUNT * pg::BATCH_MAX_COUNT];

	struct VertexData
	{
		VertexData(const glm::vec4& pos, const glm::vec4& color, int textureId, const glm::vec2& texCoords)
		{
			m_Data[pg::ATRIB_POS_X_INDEX] = pos.x;
			m_Data[pg::ATRIB_POS_Y_INDEX] = pos.y * (pg::Texture2D::FlipY() ? -1.f : 1.f);
			m_Data[pg::ATRIB_POS_Z_INDEX] = pos.z;
			m_Data[pg::ATRIB_COL_R_INDEX] = color.r;
			m_Data[pg::ATRIB_COL_G_INDEX] = color.g;
			m_Data[pg::ATRIB_COL_B_INDEX] = color.b;
			m_Data[pg::ATRIB_COL_A_INDEX] = color.a;
			m_Data[pg::ATRIB_TEX_X_INDEX] = texCoords.x;
			m_Data[pg::ATRIB_TEX_Y_INDEX] = texCoords.y;
			m_Data[pg::ATRIB_TEX_ID_INDEX] = textureId;
		}

		~VertexData() = default;

		float m_Data[pg::VERTEX_ATRIB_COUNT];
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

			memcpy(m_SquareVertices, VertexData(pos_v1, color, textureId, glm::vec2(texCoordsRect.x, texCoordsRect.y)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT], VertexData(pos_v2, color, textureId, glm::vec2(texCoordsRect.x, texCoordsRect.w)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT*2], VertexData(pos_v3, color, textureId, glm::vec2(texCoordsRect.z, texCoordsRect.w)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT*3], VertexData(pos_v4, color, textureId, glm::vec2(texCoordsRect.z, texCoordsRect.y)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));

			m_SquareIndices[0] = s_SuareIndices[0] + offsetIndices;
			m_SquareIndices[1] = s_SuareIndices[1] + offsetIndices;
			m_SquareIndices[2] = s_SuareIndices[2] + offsetIndices;
			m_SquareIndices[3] = s_SuareIndices[3] + offsetIndices;
			m_SquareIndices[4] = s_SuareIndices[4] + offsetIndices;
			m_SquareIndices[5] = s_SuareIndices[5] + offsetIndices;
		}
		~QuadData() = default;

		float m_SquareVertices[pg::VERTEX_ATRIB_COUNT*4];
		uint32_t m_SquareIndices[pg::QUAD_INDEX_COUNT];
	};

	void Init(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const pg::EngineConfigSingletonComponent& configComponent)
	{
		rendererDataComponent.m_VertexBuffer = std::move(pg::VertexBuffer::Create(s_SquareVerticesEmpty, pg::BATCH_MAX_COUNT * VERTEX_STRIDE, sizeof(float) * 10));
		rendererDataComponent.m_IndexBuffer = std::move(pg::IndexBuffer::Create(s_SuareIndicesEmpty, (pg::BATCH_MAX_COUNT * INDEX_STRIDE) / sizeof(uint32_t)));

		PG_CORE_EXCEPT(resourcesComponent.m_ShaderMap.find(configComponent.m_DefaultQuadShaderID) != resourcesComponent.m_ShaderMap.end(), "Could not find default quad shader");
		PG_CORE_EXCEPT(resourcesComponent.m_ShaderMap.find(configComponent.m_DefaultTextShaderID) != resourcesComponent.m_ShaderMap.end(), "Could not find default text shader");
		rendererDataComponent.m_QuadShader = resourcesComponent.m_ShaderMap.at(configComponent.m_DefaultQuadShaderID);
		rendererDataComponent.m_TextShader = resourcesComponent.m_ShaderMap.at(configComponent.m_DefaultTextShaderID);
	}

	void Clear(const glm::vec4& color)
	{
		pg::RenderCommand::SetClearColor(color);
		pg::RenderCommand::Clear();
	}

	void BeginScene(const pg::OrthographicCamera& ortoCamera, pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent)
	{
		pg::RenderCommand::Begin();

		rendererDataComponent.m_VertexBuffer->Bind();
		rendererDataComponent.m_IndexBuffer->Bind();
		resourcesComponent.m_TextureMap.at(resourcesComponent.m_DefaultTexture).m_Texture->Bind(0);
		rendererDataComponent.m_QuadShader->Bind();

		glm::mat4 viewProjMat = ortoCamera.GetViewProjectionMatrix();
		rendererDataComponent.m_QuadShader->UploadUniformMat4("u_ViewProjection", viewProjMat);
	}

	void Submit(unsigned int count)
	{
		pg::RenderCommand::DrawIndexed(count);
	}

	void Flush(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent)
	{
		for (auto& batch : rendererDataComponent.m_BatchMap)
		{
			auto& tex = resourcesComponent.m_TextureMap.find(batch.first);
			PG_CORE_ASSERT(tex != resourcesComponent.m_TextureMap.end(), "unable to bind texture, texture not found");
			if (tex != resourcesComponent.m_TextureMap.end())
			{
				tex->second.m_Texture->Bind(0);
				switch (tex->second.m_TextureType)
				{
				case pg::EMappedTextureType::eQuad:
					rendererDataComponent.m_QuadShader->Bind(); break;
				case pg::EMappedTextureType::eText:
					rendererDataComponent.m_TextShader->Bind(); break;
				default:
					PG_CORE_ASSERT(false, "EMappedTextureType not implemented");
				}
				rendererDataComponent.m_VertexBuffer->SetVertices(batch.second.m_VertexBuffer, batch.second.m_VertexCount, 0);
				rendererDataComponent.m_IndexBuffer->SetIndices(batch.second.m_IndexBuffer, batch.second.m_IndexCount, 0);
				Submit(batch.second.m_IndexCount);
			}
		}
		rendererDataComponent.m_BatchMap.clear();

		for (int i = 0; i < 100; i++)
		{
			auto& layer = rendererDataComponent.m_LayerBatchMap.find(i);
			if (layer != rendererDataComponent.m_LayerBatchMap.end())
			{
				std::unordered_map<pg::UUID, pg::RendererDataSingletonComponent::BatchData>& texBatches = layer->second;
				for (auto& batch : texBatches)
				{
					auto& tex = resourcesComponent.m_TextureMap.find(batch.first);
					PG_CORE_ASSERT(tex != resourcesComponent.m_TextureMap.end(), "unable to bind texture, texture not found");
					if (tex != resourcesComponent.m_TextureMap.end())
					{
						tex->second.m_Texture->Bind(0);
						switch (tex->second.m_TextureType)
						{
						case pg::EMappedTextureType::eQuad:
							rendererDataComponent.m_QuadShader->Bind(); break;
						case pg::EMappedTextureType::eText:
							rendererDataComponent.m_TextShader->Bind(); break;
						default:
							PG_CORE_ASSERT(false, "EMappedTextureType not implemented");
						}
						rendererDataComponent.m_VertexBuffer->SetVertices(batch.second.m_VertexBuffer, batch.second.m_VertexCount, 0);
						rendererDataComponent.m_IndexBuffer->SetIndices(batch.second.m_IndexBuffer, batch.second.m_IndexCount, 0);
						Submit(batch.second.m_IndexCount);
					}
				}
				texBatches.clear();
			}
		}
		rendererDataComponent.m_LayerBatchMap.clear();
	}

	void EndScene(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent)
	{
		Flush(rendererDataComponent, resourcesComponent);

		pg::RenderCommand::End();
		rendererDataComponent.m_VertexBuffer->Unbind();
		rendererDataComponent.m_IndexBuffer->Unbind();
	}
	const pg::Texture2D& GetTexture(const pg::ResourceMapSingletonComponent& resourcesComponent, const pg::UUID& textureID)
	{
		const auto it = resourcesComponent.m_TextureMap.find(textureID);
		if (it != resourcesComponent.m_TextureMap.end())
		{
			return *it->second.m_Texture.get();
		}
		else
		{
			PG_CORE_ASSERT(false, "Texture not found returning default one");

			return *resourcesComponent.m_TextureMap.at(resourcesComponent.m_DefaultTexture).m_Texture.get();
		}
	}

	void DrawLayerBatch(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const glm::vec3& col, const pg::UUID& textureID, glm::vec4 texRect, const glm::vec3& origin)
	{
		if (resourcesComponent.m_TextureMap.find(textureID) != resourcesComponent.m_TextureMap.end())
		{
			int layer = int(transform[3][2]);
			glm::mat4 finalTransform = transform;
			finalTransform[3][2] = 0;
			std::unordered_map<pg::UUID, pg::RendererDataSingletonComponent::BatchData>& texBatches = rendererDataComponent.m_LayerBatchMap[layer];
			pg::RendererDataSingletonComponent::BatchData& texBatch = texBatches[textureID];

			QuadData quad(finalTransform, glm::vec4(col, 1.f), texBatch.m_VertexCount, 0, texRect, origin);
			const unsigned int vertexBufferOffset = texBatch.m_VertexCount * pg::VERTEX_ATRIB_COUNT;
			const unsigned int indexBufferOffset = texBatch.m_IndexCount;

			PG_CORE_EXCEPT(texBatch.m_IndexCount < pg::BATCH_MAX_COUNT * pg::QUAD_INDEX_COUNT, "We are trying to allocate out of bounds, increase the limit or change the implementation to do not depend on a fixed size");
			memcpy(&texBatch.m_VertexBuffer[vertexBufferOffset], quad.m_SquareVertices, VERTEX_STRIDE);
			memcpy(&texBatch.m_IndexBuffer[indexBufferOffset], quad.m_SquareIndices, INDEX_STRIDE);

			texBatch.m_IndexCount += 6;
			texBatch.m_VertexCount += 4;
		}
		else
		{
			PG_CORE_ASSERT(false, "Texture %s not fount in renderer2d batch map", textureID.ToString().c_str());
		}
	}

	void DrawQuad(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const glm::vec3& col, const glm::vec3& origin)
	{
		DrawLayerBatch(rendererDataComponent, resourcesComponent, transform, col, resourcesComponent.m_DefaultTexture, glm::vec4(0.f, 0.f, 1.f, 1.f), origin);
	}

	void DrawSprite(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const pg::Sprite& sprite)
	{
		DrawLayerBatch(rendererDataComponent, resourcesComponent, sprite.GetTransform(), glm::vec3(1.f), sprite.GetTextureID(), sprite.GetTexCoordsRect(), sprite.GetOrigin());
	}

	void DrawString(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const std::string& string, pg::S_Ptr<pg::Font> font, const glm::vec4& color, float kerning, float linespacing)
	{
		const pg::Texture2D& fontAtlas = GetTexture(resourcesComponent, font->GetFontID());

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

				DrawLayerBatch(rendererDataComponent, resourcesComponent, charTransform, color, font->GetFontID(), texCoords, originSprite);
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

	void DrawQuad(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const pg::UUID& textureID, const glm::vec3& origin)
	{
		DrawLayerBatch(rendererDataComponent, resourcesComponent, transform, glm::vec3(1.f), textureID, glm::vec4(0.f, 0.f, 1.f, 1.f), origin);
	}

	void DrawBatch(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const glm::vec3& col, const pg::UUID& textureID, glm::vec4 texRect, const glm::vec3& origin)
	{
		if (resourcesComponent.m_TextureMap.find(textureID) != resourcesComponent.m_TextureMap.end())
		{
			pg::RendererDataSingletonComponent::BatchData& texBatch = rendererDataComponent.m_BatchMap[textureID];

			if (texBatch.m_IndexCount == pg::BATCH_MAX_COUNT * pg::QUAD_INDEX_COUNT)
			{
				Flush(rendererDataComponent, resourcesComponent);
				DrawBatch(rendererDataComponent, resourcesComponent, transform, col, textureID, texRect, origin);
			}
			else
			{
				QuadData quad(transform, glm::vec4(col, 1.f), texBatch.m_VertexCount, 0, texRect, origin);
				const unsigned int vertexBufferOffset = texBatch.m_VertexCount * pg::VERTEX_ATRIB_COUNT;
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

	void Destroy(pg::RendererDataSingletonComponent& rendererDataComponent)
	{
		rendererDataComponent.m_BatchMap.clear();
		rendererDataComponent.m_LayerBatchMap.clear();
	}

	void ProcessRenderRequests(pg::CheckedRegistryAccessor& accessor, pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent)
	{
		auto viewDrawQuad = accessor.view<const pg::DrawQuadInFrameEvent>();
		for (auto ent : viewDrawQuad)
		{
			const pg::DrawQuadInFrameEvent& event = viewDrawQuad.get<const pg::DrawQuadInFrameEvent>(ent);
			if (event.m_TextureID.IsNull())
			{
				DrawQuad(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_Color, event.m_Origin);
			}
			else
			{
				DrawQuad(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_TextureID, event.m_Origin);
			}
		}
		auto viewDrawUIQuad = accessor.view<const pg::DrawUIQuadInFrameEvent>();
		for (auto ent : viewDrawUIQuad)
		{
			const pg::DrawUIQuadInFrameEvent& event = viewDrawUIQuad.get<const pg::DrawUIQuadInFrameEvent>(ent);
			if (event.m_TextureID.IsNull())
			{
				DrawQuad(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_Color, event.m_Origin);
			}
			else
			{
				DrawQuad(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_TextureID, event.m_Origin);
			}
		}
		auto viewDrawSprite = accessor.view<const pg::DrawSpriteInFrameEvent>();
		for (auto ent : viewDrawSprite)
		{
			const pg::DrawSpriteInFrameEvent& event = viewDrawSprite.get<const pg::DrawSpriteInFrameEvent>(ent);
			
			DrawSprite(rendererDataComponent, resourcesComponent, event.m_Sprite);
		}
		auto viewDrawString = accessor.view<const pg::DrawStringInFrameEvent>();
		for (auto ent : viewDrawString)
		{
			const pg::DrawStringInFrameEvent& event = viewDrawString.get<const pg::DrawStringInFrameEvent>(ent);

			DrawString(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_String, event.m_Font, event.m_Color, event.m_Kerning, event.m_Linespacing);
		}

		auto viewDrawUIString = accessor.view<const pg::DrawUIStringInFrameEvent>();
		for (auto ent : viewDrawUIString)
		{
			const pg::DrawUIStringInFrameEvent& event = viewDrawUIString.get<const pg::DrawUIStringInFrameEvent>(ent);
			PG_CORE_EXCEPT(resourcesComponent.m_FontMap.find(event.m_FontID) != resourcesComponent.m_FontMap.end(), "Could not find font");
			DrawString(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_String, resourcesComponent.m_FontMap.at(event.m_FontID), event.m_Color, event.m_Kerning, event.m_Linespacing);
		}
	}
	void Render(pg::CheckedRegistryAccessor& accessor, const pg::OrthographicCamera& ortoCamera, pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent)
	{
		Clear({ 0.3f, 0.3f, 0.3f, 1.f });
		BeginScene(ortoCamera, rendererDataComponent, resourcesComponent);
		ProcessRenderRequests(accessor, rendererDataComponent, resourcesComponent);
		EndScene(rendererDataComponent, resourcesComponent);
	}
}

pg::SystemAccessDecl pg::Renderer2DSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.addSet = {
		std::type_index(typeid(pg::RendererDataSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::RendererDataSingletonComponent)),
	};
	decl.readSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
		std::type_index(typeid(pg::OrthographicCameraComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::DrawQuadInFrameEvent)),
		std::type_index(typeid(pg::DrawSpriteInFrameEvent)),
		std::type_index(typeid(pg::DrawStringInFrameEvent)),
		std::type_index(typeid(pg::DrawUIQuadInFrameEvent)),
		std::type_index(typeid(pg::DrawUIStringInFrameEvent)),
	};
	return decl;
}

void pg::Renderer2DSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto rendererDataView = accessor.view<pg::RendererDataSingletonComponent>();
	auto viewCamera = accessor.view<const pg::OrthographicCameraComponent>();
	auto resourcesView = accessor.view<const pg::ResourceMapSingletonComponent>();
	auto configView = accessor.view<const pg::EngineConfigSingletonComponent>();
	if (viewCamera.empty() || resourcesView.empty() || configView.empty())
	{
		return;
	}
	const pg::OrthographicCamera& ortoCamera = viewCamera.get<const pg::OrthographicCameraComponent>(viewCamera.front()).m_Camera;
	const pg::ResourceMapSingletonComponent& resourcesComponent = resourcesView.get<const pg::ResourceMapSingletonComponent>(resourcesView.front());
	const pg::EngineConfigSingletonComponent& configComponent = configView.get<const pg::EngineConfigSingletonComponent>(configView.front());
	if (rendererDataView.empty())
	{
		pg::RendererDataSingletonComponent rendererDataComponent;
		entt::entity singletonEntity = accessor.create();
		Init(rendererDataComponent, resourcesComponent, configComponent);
		Render(accessor, ortoCamera, rendererDataComponent, resourcesComponent);
		accessor.emplace_deferred<pg::RendererDataSingletonComponent>(singletonEntity, std::move(rendererDataComponent));
	}
	else
	{
		pg::RendererDataSingletonComponent& rendererDataComponent = rendererDataView.get<pg::RendererDataSingletonComponent>(rendererDataView.front());
		Render(accessor, ortoCamera, rendererDataComponent, resourcesComponent);
	}
}


