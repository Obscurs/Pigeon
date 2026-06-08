#include "pch.h"
#include "Pigeon/Renderer/Renderer2DSystem.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Pigeon/Renderer/DrawStringInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIStringInFrameEvent.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/MSDFData.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Renderer/RenderCommand.h"
#include "Pigeon/Renderer/RendererDataSingletonComponent.h"
#include "Pigeon/Renderer/Texture.h"

#include <algorithm>
#include <memory>
#include <vector>

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

			const glm::vec4 posV1 = combinedTransform * glm::vec4(0, 0, 0, 1);
			const glm::vec4 posV2 = combinedTransform * glm::vec4(0, 1, 0, 1);
			const glm::vec4 posV3 = combinedTransform * glm::vec4(1, 1, 0, 1);
			const glm::vec4 posV4 = combinedTransform * glm::vec4(1, 0, 0, 1);

			memcpy(m_SquareVertices, VertexData(posV1, color, textureId, glm::vec2(texCoordsRect.x, texCoordsRect.y)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT], VertexData(posV2, color, textureId, glm::vec2(texCoordsRect.x, texCoordsRect.w)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT*2], VertexData(posV3, color, textureId, glm::vec2(texCoordsRect.z, texCoordsRect.w)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT*3], VertexData(posV4, color, textureId, glm::vec2(texCoordsRect.z, texCoordsRect.y)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));

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

	const pg::Texture2D& GetTexture(const pg::ResourceMapSingletonComponent& resourcesComponent, const pg::UUID& textureID);

	// A single quad ready to draw: four transformed vertices, the texture it samples, and the world
	// draw-order key. Items are sorted by m_SortKey (lower draws behind) before being batched.
	struct DrawItem
	{
		float m_Vertices[pg::VERTEX_ATRIB_COUNT * pg::QUAD_VERTEX_COUNT];
		pg::UUID m_TextureID;
		float m_SortKey = 0.f;
	};

	void FlushBatch(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, pg::RendererDataSingletonComponent::BatchData& batch, const pg::UUID& textureID)
	{
		if (batch.m_IndexCount == 0)
		{
			return;
		}
		const std::unordered_map<pg::UUID, pg::MappedTexture>::const_iterator tex = resourcesComponent.m_TextureMap.find(textureID);
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
			rendererDataComponent.m_VertexBuffer->SetVertices(batch.m_VertexBuffer, batch.m_VertexCount, 0);
			rendererDataComponent.m_IndexBuffer->SetIndices(batch.m_IndexBuffer, batch.m_IndexCount, 0);
			Submit(batch.m_IndexCount);
		}
		batch.m_VertexCount = 0;
		batch.m_IndexCount = 0;
	}

	// Draws items strictly in list order, merging runs of the same texture into one draw call and
	// flushing whenever the texture changes or the batch fills. Order in equals draw order out, so the
	// caller controls layering by ordering the list.
	void DrawOrderedItems(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const std::vector<DrawItem>& items)
	{
		if (items.empty())
		{
			return;
		}
		std::unique_ptr<pg::RendererDataSingletonComponent::BatchData> batch = std::make_unique<pg::RendererDataSingletonComponent::BatchData>();
		pg::UUID currentTexture = items.front().m_TextureID;
		for (const DrawItem& item : items)
		{
			const bool batchFull = batch->m_IndexCount >= pg::BATCH_MAX_COUNT * pg::QUAD_INDEX_COUNT;
			if (item.m_TextureID != currentTexture || batchFull)
			{
				FlushBatch(rendererDataComponent, resourcesComponent, *batch, currentTexture);
				currentTexture = item.m_TextureID;
			}
			const unsigned int vertexOffset = batch->m_VertexCount * pg::VERTEX_ATRIB_COUNT;
			memcpy(&batch->m_VertexBuffer[vertexOffset], item.m_Vertices, pg::VERTEX_ATRIB_COUNT * pg::QUAD_VERTEX_COUNT * sizeof(float));
			for (unsigned int i = 0; i < pg::QUAD_INDEX_COUNT; ++i)
			{
				batch->m_IndexBuffer[batch->m_IndexCount + i] = s_SuareIndices[i] + batch->m_VertexCount;
			}
			batch->m_VertexCount += pg::QUAD_VERTEX_COUNT;
			batch->m_IndexCount += pg::QUAD_INDEX_COUNT;
		}
		FlushBatch(rendererDataComponent, resourcesComponent, *batch, currentTexture);
	}

	// Builds a quad's transformed vertices and appends it as a draw item. The translation z is dropped:
	// this is a 2D renderer, depth never affects geometry — world draw order comes from m_SortKey.
	void AppendQuad(std::vector<DrawItem>& items, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const glm::vec4& color, const pg::UUID& textureID, const glm::vec4& texCoordsRect, const glm::vec3& origin, float sortKey)
	{
		if (resourcesComponent.m_TextureMap.find(textureID) == resourcesComponent.m_TextureMap.end())
		{
			PG_CORE_ASSERT(false, "Texture {0} not found in renderer2d batch map", textureID.ToString());
			return;
		}
		glm::mat4 flatTransform = transform;
		flatTransform[3][2] = 0.f;
		QuadData quad(flatTransform, color, 0, 0, texCoordsRect, origin);

		DrawItem item;
		memcpy(item.m_Vertices, quad.m_SquareVertices, pg::VERTEX_ATRIB_COUNT * pg::QUAD_VERTEX_COUNT * sizeof(float));
		item.m_TextureID = textureID;
		item.m_SortKey = sortKey;
		items.push_back(item);
	}

	void AppendString(std::vector<DrawItem>& items, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const std::string& string, pg::S_Ptr<pg::Font> font, const glm::vec4& color, float kerning, float linespacing, float sortKey)
	{
		const pg::Texture2D& fontAtlas = GetTexture(resourcesComponent, font->GetFontID());

		glm::mat4 flatTransform = transform;
		flatTransform[3][2] = 0.f;

		glm::dvec2 charOffset{ 0.0, 0.0 };
		const glm::vec3 originSprite(0.f, 0.0f, 0.f);

		for (size_t i = 0; i < string.size(); i++)
		{
			char character = string[i];

			if (font->IsCharacterDrawable(character))
			{
				const glm::vec4 texCoords = font->GetCharacterTexCoordsQuad(character, fontAtlas);
				const glm::vec4 charQuad = font->GetCharacterVertexQuad(character, charOffset);
				const glm::mat4 charTransform = font->GetCharacterTransform(charQuad, flatTransform);

				AppendQuad(items, resourcesComponent, charTransform, color, font->GetFontID(), texCoords, originSprite, sortKey);
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

	void EndScene(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent)
	{
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

	void ProcessRenderRequests(pg::CheckedRegistryAccessor& accessor, pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent)
	{
		std::vector<DrawItem> worldItems;
		std::vector<DrawItem> uiItems;

		auto viewDrawQuad = accessor.View<const pg::DrawQuadInFrameEvent>();
		for (auto ent : viewDrawQuad)
		{
			const pg::DrawQuadInFrameEvent& event = viewDrawQuad.get<const pg::DrawQuadInFrameEvent>(ent);
			const pg::UUID textureID = event.m_TextureID.IsNull() ? resourcesComponent.m_DefaultTexture : event.m_TextureID;
			const glm::vec4 color = event.m_TextureID.IsNull() ? glm::vec4(event.m_Color, 1.f) : glm::vec4(1.f);
			AppendQuad(worldItems, resourcesComponent, event.m_Transform, color, textureID, glm::vec4(0.f, 0.f, 1.f, 1.f), event.m_Origin, event.m_SortKey);
		}

		auto viewDrawSprite = accessor.View<const pg::DrawSpriteInFrameEvent>();
		for (auto ent : viewDrawSprite)
		{
			const pg::DrawSpriteInFrameEvent& event = viewDrawSprite.get<const pg::DrawSpriteInFrameEvent>(ent);
			const pg::Sprite& sprite = event.m_Sprite;
			AppendQuad(worldItems, resourcesComponent, sprite.GetTransform(), glm::vec4(1.f), sprite.GetTextureID(), sprite.GetTexCoordsRect(), sprite.GetOrigin(), event.m_SortKey);
		}

		auto viewDrawString = accessor.View<const pg::DrawStringInFrameEvent>();
		for (auto ent : viewDrawString)
		{
			const pg::DrawStringInFrameEvent& event = viewDrawString.get<const pg::DrawStringInFrameEvent>(ent);
			AppendString(worldItems, resourcesComponent, event.m_Transform, event.m_String, event.m_Font, event.m_Color, event.m_Kerning, event.m_Linespacing, event.m_SortKey);
		}

		// UI elements keep their nesting level packed into the transform z; it is used purely to order
		// the UI pass among itself. UI is never part of the world Y-sort.
		auto viewDrawUIQuad = accessor.View<const pg::DrawUIQuadInFrameEvent>();
		for (auto ent : viewDrawUIQuad)
		{
			const pg::DrawUIQuadInFrameEvent& event = viewDrawUIQuad.get<const pg::DrawUIQuadInFrameEvent>(ent);
			const pg::UUID textureID = event.m_TextureID.IsNull() ? resourcesComponent.m_DefaultTexture : event.m_TextureID;
			const glm::vec4 color = event.m_TextureID.IsNull() ? glm::vec4(event.m_Color, 1.f) : glm::vec4(1.f);
			AppendQuad(uiItems, resourcesComponent, event.m_Transform, color, textureID, glm::vec4(0.f, 0.f, 1.f, 1.f), event.m_Origin, event.m_Transform[3][2]);
		}

		auto viewDrawUIString = accessor.View<const pg::DrawUIStringInFrameEvent>();
		for (auto ent : viewDrawUIString)
		{
			const pg::DrawUIStringInFrameEvent& event = viewDrawUIString.get<const pg::DrawUIStringInFrameEvent>(ent);
			PG_CORE_EXCEPT(resourcesComponent.m_FontMap.find(event.m_FontID) != resourcesComponent.m_FontMap.end(), "Could not find font");
			AppendString(uiItems, resourcesComponent, event.m_Transform, event.m_String, resourcesComponent.m_FontMap.at(event.m_FontID), event.m_Color, event.m_Kerning, event.m_Linespacing, event.m_Transform[3][2]);
		}

		std::stable_sort(worldItems.begin(), worldItems.end(),
			[](const DrawItem& lhs, const DrawItem& rhs) { return lhs.m_SortKey < rhs.m_SortKey; });
		std::stable_sort(uiItems.begin(), uiItems.end(),
			[](const DrawItem& lhs, const DrawItem& rhs) { return lhs.m_SortKey < rhs.m_SortKey; });

		// UI is appended after every world item so it always draws on top.
		worldItems.insert(worldItems.end(), uiItems.begin(), uiItems.end());
		DrawOrderedItems(rendererDataComponent, resourcesComponent, worldItems);
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

	auto rendererDataView = accessor.View<pg::RendererDataSingletonComponent>();
	auto viewCamera = accessor.View<const pg::OrthographicCameraComponent>();
	auto resourcesView = accessor.View<const pg::ResourceMapSingletonComponent>();
	auto configView = accessor.View<const pg::EngineConfigSingletonComponent>();
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
		pg::ecs::Entity singletonEntity = accessor.Create();
		Init(rendererDataComponent, resourcesComponent, configComponent);
		Render(accessor, ortoCamera, rendererDataComponent, resourcesComponent);
		accessor.EmplaceDeferred<pg::RendererDataSingletonComponent>(singletonEntity, std::move(rendererDataComponent));
	}
	else
	{
		pg::RendererDataSingletonComponent& rendererDataComponent = rendererDataView.get<pg::RendererDataSingletonComponent>(rendererDataView.front());
		Render(accessor, ortoCamera, rendererDataComponent, resourcesComponent);
	}
}


