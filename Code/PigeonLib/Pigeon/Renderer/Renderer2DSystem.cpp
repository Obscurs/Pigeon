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
	static const unsigned int VERTEX_STRIDE = pig::VERTEX_ATRIB_COUNT * pig::QUAD_VERTEX_COUNT * sizeof(float);
	static const unsigned int INDEX_STRIDE = pig::QUAD_INDEX_COUNT * sizeof(int);

	static const uint32_t s_SuareIndices[pig::QUAD_INDEX_COUNT] = { 0, 2, 1, 2, 0, 3 };

	static const float s_SquareVerticesEmpty[pig::VERTEX_ATRIB_COUNT * pig::QUAD_VERTEX_COUNT * pig::BATCH_MAX_COUNT];
	static const uint32_t s_SuareIndicesEmpty[pig::QUAD_INDEX_COUNT * pig::BATCH_MAX_COUNT];

	struct VertexData
	{
		VertexData(const glm::vec4& pos, const glm::vec4& color, int textureId, const glm::vec2& texCoords)
		{
			m_Data[pig::ATRIB_POS_X_INDEX] = pos.x;
			m_Data[pig::ATRIB_POS_Y_INDEX] = pos.y * (pig::Texture2D::FlipY() ? -1.f : 1.f);
			m_Data[pig::ATRIB_POS_Z_INDEX] = pos.z;
			m_Data[pig::ATRIB_COL_R_INDEX] = color.r;
			m_Data[pig::ATRIB_COL_G_INDEX] = color.g;
			m_Data[pig::ATRIB_COL_B_INDEX] = color.b;
			m_Data[pig::ATRIB_COL_A_INDEX] = color.a;
			m_Data[pig::ATRIB_TEX_X_INDEX] = texCoords.x;
			m_Data[pig::ATRIB_TEX_Y_INDEX] = texCoords.y;
			m_Data[pig::ATRIB_TEX_ID_INDEX] = textureId;
		}

		~VertexData() = default;

		float m_Data[pig::VERTEX_ATRIB_COUNT];
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

			memcpy(m_SquareVertices, VertexData(pos_v1, color, textureId, glm::vec2(texCoordsRect.x, texCoordsRect.y)).m_Data, pig::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::VERTEX_ATRIB_COUNT], VertexData(pos_v2, color, textureId, glm::vec2(texCoordsRect.x, texCoordsRect.w)).m_Data, pig::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::VERTEX_ATRIB_COUNT*2], VertexData(pos_v3, color, textureId, glm::vec2(texCoordsRect.z, texCoordsRect.w)).m_Data, pig::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pig::VERTEX_ATRIB_COUNT*3], VertexData(pos_v4, color, textureId, glm::vec2(texCoordsRect.z, texCoordsRect.y)).m_Data, pig::VERTEX_ATRIB_COUNT * sizeof(float));

			m_SquareIndices[0] = s_SuareIndices[0] + offsetIndices;
			m_SquareIndices[1] = s_SuareIndices[1] + offsetIndices;
			m_SquareIndices[2] = s_SuareIndices[2] + offsetIndices;
			m_SquareIndices[3] = s_SuareIndices[3] + offsetIndices;
			m_SquareIndices[4] = s_SuareIndices[4] + offsetIndices;
			m_SquareIndices[5] = s_SuareIndices[5] + offsetIndices;
		}
		~QuadData() = default;

		float m_SquareVertices[pig::VERTEX_ATRIB_COUNT*4];
		uint32_t m_SquareIndices[pig::QUAD_INDEX_COUNT];
	};

	void Init(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent, const pig::EngineConfigSingletonComponent& configComponent)
	{
		rendererDataComponent.m_VertexBuffer = std::move(pig::VertexBuffer::Create(s_SquareVerticesEmpty, pig::BATCH_MAX_COUNT * VERTEX_STRIDE, sizeof(float) * 10));
		rendererDataComponent.m_IndexBuffer = std::move(pig::IndexBuffer::Create(s_SuareIndicesEmpty, (pig::BATCH_MAX_COUNT * INDEX_STRIDE) / sizeof(uint32_t)));

		PG_CORE_EXCEPT(resourcesComponent.m_ShaderMap.find(configComponent.m_DefaultQuadShaderID) != resourcesComponent.m_ShaderMap.end(), "Could not find default quad shader");
		PG_CORE_EXCEPT(resourcesComponent.m_ShaderMap.find(configComponent.m_DefaultTextShaderID) != resourcesComponent.m_ShaderMap.end(), "Could not find default text shader");
		rendererDataComponent.m_QuadShader = resourcesComponent.m_ShaderMap.at(configComponent.m_DefaultQuadShaderID);
		rendererDataComponent.m_TextShader = resourcesComponent.m_ShaderMap.at(configComponent.m_DefaultTextShaderID);
	}

	void Clear(const glm::vec4& color)
	{
		pig::RenderCommand::SetClearColor(color);
		pig::RenderCommand::Clear();
	}

	void BeginScene(const pig::OrthographicCamera& ortoCamera, pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent)
	{
		pig::RenderCommand::Begin();

		rendererDataComponent.m_VertexBuffer->Bind();
		rendererDataComponent.m_IndexBuffer->Bind();
		resourcesComponent.m_TextureMap.at(resourcesComponent.m_DefaultTexture).m_Texture->Bind(0);
		rendererDataComponent.m_QuadShader->Bind();

		glm::mat4 viewProjMat = ortoCamera.GetViewProjectionMatrix();
		rendererDataComponent.m_QuadShader->UploadUniformMat4("u_ViewProjection", viewProjMat);
	}

	void Submit(unsigned int count)
	{
		pig::RenderCommand::DrawIndexed(count);
	}

	void Flush(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent)
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
				case pig::EMappedTextureType::eQuad:
					rendererDataComponent.m_QuadShader->Bind(); break;
				case pig::EMappedTextureType::eText:
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
				std::unordered_map<pig::UUID, pig::RendererDataSingletonComponent::BatchData>& texBatches = layer->second;
				for (auto& batch : texBatches)
				{
					auto& tex = resourcesComponent.m_TextureMap.find(batch.first);
					PG_CORE_ASSERT(tex != resourcesComponent.m_TextureMap.end(), "unable to bind texture, texture not found");
					if (tex != resourcesComponent.m_TextureMap.end())
					{
						tex->second.m_Texture->Bind(0);
						switch (tex->second.m_TextureType)
						{
						case pig::EMappedTextureType::eQuad:
							rendererDataComponent.m_QuadShader->Bind(); break;
						case pig::EMappedTextureType::eText:
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

	void EndScene(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent)
	{
		Flush(rendererDataComponent, resourcesComponent);

		pig::RenderCommand::End();
		rendererDataComponent.m_VertexBuffer->Unbind();
		rendererDataComponent.m_IndexBuffer->Unbind();
	}
	const pig::Texture2D& GetTexture(const pig::ResourceMapSingletonComponent& resourcesComponent, const pig::UUID& textureID)
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

	void DrawLayerBatch(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const glm::vec3& col, const pig::UUID& textureID, glm::vec4 texRect, const glm::vec3& origin)
	{
		if (resourcesComponent.m_TextureMap.find(textureID) != resourcesComponent.m_TextureMap.end())
		{
			int layer = int(transform[3][2]);
			glm::mat4 finalTransform = transform;
			finalTransform[3][2] = 0;
			std::unordered_map<pig::UUID, pig::RendererDataSingletonComponent::BatchData>& texBatches = rendererDataComponent.m_LayerBatchMap[layer];
			pig::RendererDataSingletonComponent::BatchData& texBatch = texBatches[textureID];

			QuadData quad(finalTransform, glm::vec4(col, 1.f), texBatch.m_VertexCount, 0, texRect, origin);
			const unsigned int vertexBufferOffset = texBatch.m_VertexCount * pig::VERTEX_ATRIB_COUNT;
			const unsigned int indexBufferOffset = texBatch.m_IndexCount;

			PG_CORE_EXCEPT(texBatch.m_IndexCount < pig::BATCH_MAX_COUNT * pig::QUAD_INDEX_COUNT, "We are trying to allocate out of bounds, increase the limit or change the implementation to do not depend on a fixed size");
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

	void DrawQuad(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const glm::vec3& col, const glm::vec3& origin)
	{
		DrawLayerBatch(rendererDataComponent, resourcesComponent, transform, col, resourcesComponent.m_DefaultTexture, glm::vec4(0.f, 0.f, 1.f, 1.f), origin);
	}

	void DrawSprite(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent, const pig::Sprite& sprite)
	{
		DrawLayerBatch(rendererDataComponent, resourcesComponent, sprite.GetTransform(), glm::vec3(1.f), sprite.GetTextureID(), sprite.GetTexCoordsRect(), sprite.GetOrigin());
	}

	void DrawString(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const std::string& string, pig::S_Ptr<pig::Font> font, const glm::vec4& color, float kerning, float linespacing)
	{
		const pig::Texture2D& fontAtlas = GetTexture(resourcesComponent, font->GetFontID());

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

	void DrawQuad(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const pig::UUID& textureID, const glm::vec3& origin)
	{
		DrawLayerBatch(rendererDataComponent, resourcesComponent, transform, glm::vec3(1.f), textureID, glm::vec4(0.f, 0.f, 1.f, 1.f), origin);
	}

	void DrawBatch(pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const glm::vec3& col, const pig::UUID& textureID, glm::vec4 texRect, const glm::vec3& origin)
	{
		if (resourcesComponent.m_TextureMap.find(textureID) != resourcesComponent.m_TextureMap.end())
		{
			pig::RendererDataSingletonComponent::BatchData& texBatch = rendererDataComponent.m_BatchMap[textureID];

			if (texBatch.m_IndexCount == pig::BATCH_MAX_COUNT * pig::QUAD_INDEX_COUNT)
			{
				Flush(rendererDataComponent, resourcesComponent);
				DrawBatch(rendererDataComponent, resourcesComponent, transform, col, textureID, texRect, origin);
			}
			else
			{
				QuadData quad(transform, glm::vec4(col, 1.f), texBatch.m_VertexCount, 0, texRect, origin);
				const unsigned int vertexBufferOffset = texBatch.m_VertexCount * pig::VERTEX_ATRIB_COUNT;
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

	void Destroy(pig::RendererDataSingletonComponent& rendererDataComponent)
	{
		rendererDataComponent.m_BatchMap.clear();
		rendererDataComponent.m_LayerBatchMap.clear();
	}

	void ProcessRenderRequests(pig::CheckedRegistryAccessor& accessor, pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent)
	{
		auto viewDrawQuad = accessor.view<const pig::DrawQuadInFrameEvent>();
		for (auto ent : viewDrawQuad)
		{
			const pig::DrawQuadInFrameEvent& event = viewDrawQuad.get<const pig::DrawQuadInFrameEvent>(ent);
			if (event.m_TextureID.IsNull())
			{
				DrawQuad(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_Color, event.m_Origin);
			}
			else
			{
				DrawQuad(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_TextureID, event.m_Origin);
			}
		}
		auto viewDrawUIQuad = accessor.view<const pig::DrawUIQuadInFrameEvent>();
		for (auto ent : viewDrawUIQuad)
		{
			const pig::DrawUIQuadInFrameEvent& event = viewDrawUIQuad.get<const pig::DrawUIQuadInFrameEvent>(ent);
			if (event.m_TextureID.IsNull())
			{
				DrawQuad(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_Color, event.m_Origin);
			}
			else
			{
				DrawQuad(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_TextureID, event.m_Origin);
			}
		}
		auto viewDrawSprite = accessor.view<const pig::DrawSpriteInFrameEvent>();
		for (auto ent : viewDrawSprite)
		{
			const pig::DrawSpriteInFrameEvent& event = viewDrawSprite.get<const pig::DrawSpriteInFrameEvent>(ent);
			
			DrawSprite(rendererDataComponent, resourcesComponent, event.m_Sprite);
		}
		auto viewDrawString = accessor.view<const pig::DrawStringInFrameEvent>();
		for (auto ent : viewDrawString)
		{
			const pig::DrawStringInFrameEvent& event = viewDrawString.get<const pig::DrawStringInFrameEvent>(ent);

			DrawString(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_String, event.m_Font, event.m_Color, event.m_Kerning, event.m_Linespacing);
		}

		auto viewDrawUIString = accessor.view<const pig::DrawUIStringInFrameEvent>();
		for (auto ent : viewDrawUIString)
		{
			const pig::DrawUIStringInFrameEvent& event = viewDrawUIString.get<const pig::DrawUIStringInFrameEvent>(ent);
			PG_CORE_EXCEPT(resourcesComponent.m_FontMap.find(event.m_FontID) != resourcesComponent.m_FontMap.end(), "Could not find font");
			DrawString(rendererDataComponent, resourcesComponent, event.m_Transform, event.m_String, resourcesComponent.m_FontMap.at(event.m_FontID), event.m_Color, event.m_Kerning, event.m_Linespacing);
		}
	}
	void Render(pig::CheckedRegistryAccessor& accessor, const pig::OrthographicCamera& ortoCamera, pig::RendererDataSingletonComponent& rendererDataComponent, const pig::ResourceMapSingletonComponent& resourcesComponent)
	{
		Clear({ 0.3f, 0.3f, 0.3f, 1.f });
		BeginScene(ortoCamera, rendererDataComponent, resourcesComponent);
		ProcessRenderRequests(accessor, rendererDataComponent, resourcesComponent);
		EndScene(rendererDataComponent, resourcesComponent);
	}
}

pig::SystemAccessDecl pig::Renderer2DSystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;
	decl.addSet = {
		std::type_index(typeid(pig::RendererDataSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pig::RendererDataSingletonComponent)),
	};
	decl.readSet = {
		std::type_index(typeid(pig::EngineConfigSingletonComponent)),
		std::type_index(typeid(pig::OrthographicCameraComponent)),
		std::type_index(typeid(pig::ResourceMapSingletonComponent)),
		std::type_index(typeid(pig::DrawQuadInFrameEvent)),
		std::type_index(typeid(pig::DrawSpriteInFrameEvent)),
		std::type_index(typeid(pig::DrawStringInFrameEvent)),
		std::type_index(typeid(pig::DrawUIQuadInFrameEvent)),
		std::type_index(typeid(pig::DrawUIStringInFrameEvent)),
	};
	return decl;
}

void pig::Renderer2DSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	auto rendererDataView = accessor.view<pig::RendererDataSingletonComponent>();
	auto viewCamera = accessor.view<const pig::OrthographicCameraComponent>();
	auto resourcesView = accessor.view<const pig::ResourceMapSingletonComponent>();
	auto configView = accessor.view<const pig::EngineConfigSingletonComponent>();
	if (viewCamera.empty() || resourcesView.empty() || configView.empty())
	{
		return;
	}
	const pig::OrthographicCamera& ortoCamera = viewCamera.get<const pig::OrthographicCameraComponent>(viewCamera.front()).m_Camera;
	const pig::ResourceMapSingletonComponent& resourcesComponent = resourcesView.get<const pig::ResourceMapSingletonComponent>(resourcesView.front());
	const pig::EngineConfigSingletonComponent& configComponent = configView.get<const pig::EngineConfigSingletonComponent>(configView.front());
	if (rendererDataView.empty())
	{
		pig::RendererDataSingletonComponent rendererDataComponent;
		entt::entity singletonEntity = accessor.create();
		Init(rendererDataComponent, resourcesComponent, configComponent);
		Render(accessor, ortoCamera, rendererDataComponent, resourcesComponent);
		accessor.emplace_deferred<pig::RendererDataSingletonComponent>(singletonEntity, std::move(rendererDataComponent));
	}
	else
	{
		pig::RendererDataSingletonComponent& rendererDataComponent = rendererDataView.get<pig::RendererDataSingletonComponent>(rendererDataView.front());
		Render(accessor, ortoCamera, rendererDataComponent, resourcesComponent);
	}
}


