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
#include "Pigeon/Renderer/Renderer3DDataSingletonComponent.h"
#include "Pigeon/Renderer/RendererDataSingletonComponent.h"
#include "Pigeon/Renderer/Texture.h"
#include "Pigeon/Renderer/UICameraSingletonComponent.h"

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

	// World space is Y-up: +Y is up on screen, matching the gameplay/transform convention. DirectX
	// samples textures with V increasing downward, so a world (Y-up) quad would sample its texture
	// upside down; world draws compensate by flipping the V texture coordinate (see QuadData's
	// flipTexV). The position is never negated here — the camera projections map world/UI space to
	// the screen directly.
	struct VertexData
	{
		VertexData(const glm::vec4& pos, const glm::vec4& color, int textureId, const glm::vec2& texCoords)
		{
			m_Data[pg::ATRIB_POS_X_INDEX] = pos.x;
			m_Data[pg::ATRIB_POS_Y_INDEX] = pos.y;
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
		// flipTexV swaps the rect's top/bottom V so a Y-up world quad samples its texture upright on
		// DirectX (V-down). World draws pass it; UI draws (y-down canvas) do not.
		QuadData(const glm::mat4& transform, const glm::vec4& color, unsigned int offsetIndices, int textureId, const glm::vec4& texCoordsRect, const glm::vec3& origin, bool flipTexV)
		{
			const glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -origin);
			const glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), origin);
			const glm::mat4 combinedTransform = translateBack * transform * translateToOrigin;

			const glm::vec4 posV1 = combinedTransform * glm::vec4(0, 0, 0, 1);
			const glm::vec4 posV2 = combinedTransform * glm::vec4(0, 1, 0, 1);
			const glm::vec4 posV3 = combinedTransform * glm::vec4(1, 1, 0, 1);
			const glm::vec4 posV4 = combinedTransform * glm::vec4(1, 0, 0, 1);

			const float vBottom = flipTexV ? texCoordsRect.w : texCoordsRect.y;
			const float vTop = flipTexV ? texCoordsRect.y : texCoordsRect.w;

			memcpy(m_SquareVertices, VertexData(posV1, color, textureId, glm::vec2(texCoordsRect.x, vBottom)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT], VertexData(posV2, color, textureId, glm::vec2(texCoordsRect.x, vTop)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT*2], VertexData(posV3, color, textureId, glm::vec2(texCoordsRect.z, vTop)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));
			memcpy(&m_SquareVertices[pg::VERTEX_ATRIB_COUNT*3], VertexData(posV4, color, textureId, glm::vec2(texCoordsRect.z, vBottom)).m_Data, pg::VERTEX_ATRIB_COUNT * sizeof(float));

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
		glm::vec4 m_ClipRect{ 0.f, 0.f, 0.f, 0.f };  // window-pixel scissor rect; applied only in the UI pass
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
	// Applies the item's clip rect as a window-pixel scissor; used in the UI pass so a clip change flushes
	// the current batch and narrows the scissor, exactly like a texture change.
	void ApplyScissor(const glm::vec4& clipRect)
	{
		pg::RenderCommand::SetScissor(static_cast<int>(clipRect.x), static_cast<int>(clipRect.y), static_cast<int>(clipRect.z), static_cast<int>(clipRect.w));
	}

	void DrawOrderedItems(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent, const std::vector<DrawItem>& items, bool applyScissor)
	{
		if (items.empty())
		{
			return;
		}
		std::unique_ptr<pg::RendererDataSingletonComponent::BatchData> batch = std::make_unique<pg::RendererDataSingletonComponent::BatchData>();
		pg::UUID currentTexture = items.front().m_TextureID;
		glm::vec4 currentClip = items.front().m_ClipRect;
		if (applyScissor)
		{
			ApplyScissor(currentClip);
		}
		for (const DrawItem& item : items)
		{
			const bool batchFull = batch->m_IndexCount >= pg::BATCH_MAX_COUNT * pg::QUAD_INDEX_COUNT;
			const bool clipChanged = applyScissor && item.m_ClipRect != currentClip;
			if (item.m_TextureID != currentTexture || batchFull || clipChanged)
			{
				FlushBatch(rendererDataComponent, resourcesComponent, *batch, currentTexture);
				currentTexture = item.m_TextureID;
				if (clipChanged)
				{
					currentClip = item.m_ClipRect;
					ApplyScissor(currentClip);
				}
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
	void AppendQuad(std::vector<DrawItem>& items, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const glm::vec4& color, const pg::UUID& textureID, const glm::vec4& texCoordsRect, const glm::vec3& origin, float sortKey, const glm::vec4& clipRect, bool flipTexV)
	{
		// A non-null UUID can legitimately be absent until it is registered at runtime (e.g. a Generated
		// Texture before its diffusion job completes, ADR 0008); fall back to the default texture
		// meanwhile so the quad still draws instead of asserting.
		pg::UUID resolvedTextureID = textureID;
		if (resourcesComponent.m_TextureMap.find(resolvedTextureID) == resourcesComponent.m_TextureMap.end())
		{
			resolvedTextureID = resourcesComponent.m_DefaultTexture;
		}
		glm::mat4 flatTransform = transform;
		flatTransform[3][2] = 0.f;
		QuadData quad(flatTransform, color, 0, 0, texCoordsRect, origin, flipTexV);

		DrawItem item;
		memcpy(item.m_Vertices, quad.m_SquareVertices, pg::VERTEX_ATRIB_COUNT * pg::QUAD_VERTEX_COUNT * sizeof(float));
		item.m_TextureID = resolvedTextureID;
		item.m_SortKey = sortKey;
		item.m_ClipRect = clipRect;
		items.push_back(item);
	}

	// Reflects a transform about the horizontal line through its own anchor (translation), i.e.
	// flips the Y of everything the transform places while leaving the anchor point fixed.
	glm::mat4 ReflectAboutAnchorY(const glm::mat4& transform)
	{
		const glm::vec3 anchor(transform[3]);
		const glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.f, -1.f, 1.f));
		return glm::translate(glm::mat4(1.0f), anchor) * flipY * glm::translate(glm::mat4(1.0f), -anchor) * transform;
	}

	// reflectAnchorY handles world (Y-up) text: the font lays glyphs out top-to-bottom in a y-down
	// local space, so a world string is reflected about its anchor to read top-to-bottom on a Y-up
	// screen. This keeps glyph textures upright without a per-glyph V flip. UI text (y-down canvas)
	// passes false and is laid out directly.
	void AppendString(std::vector<DrawItem>& items, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::mat4& transform, const std::string& string, pg::S_Ptr<pg::Font> font, const glm::vec4& color, float kerning, float linespacing, float sortKey, const glm::vec4& clipRect, int visibleChars, bool reflectAnchorY)
	{
		const pg::Texture2D& fontAtlas = GetTexture(resourcesComponent, font->GetFontID());

		glm::mat4 flatTransform = transform;
		flatTransform[3][2] = 0.f;
		if (reflectAnchorY)
		{
			flatTransform = ReflectAboutAnchorY(flatTransform);
		}

		glm::dvec2 charOffset{ 0.0, 0.0 };
		const glm::vec3 originSprite(0.f, 0.0f, 0.f);

		for (size_t i = 0; i < string.size(); i++)
		{
			// Typewriter reveal: stop once we reach the first hidden source character. The layout was
			// produced for the full string, so the glyphs drawn here keep their final positions.
			if (visibleChars >= 0 && static_cast<int>(i) >= visibleChars)
				break;

			char character = string[i];

			if (font->IsCharacterDrawable(character))
			{
				const glm::vec4 texCoords = font->GetCharacterTexCoordsQuad(character, fontAtlas);
				const glm::vec4 charQuad = font->GetCharacterVertexQuad(character, charOffset);
				const glm::mat4 charTransform = font->GetCharacterTransform(charQuad, flatTransform);

				// Glyphs are positioned by flatTransform (already reflected for world text); their
				// textures are not V-flipped — the reflection keeps them upright.
				AppendQuad(items, resourcesComponent, charTransform, color, font->GetFontID(), texCoords, originSprite, sortKey, clipRect, false);
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

	// Collects every in-frame draw request into two ordered lists: world items (Y-sorted) and UI items
	// (ordered by their packed nesting level). The two lists are drawn in separate passes so UI can use
	// the screen-space UI camera and always render on top of the world.
	void CollectDrawItems(pg::CheckedRegistryAccessor& accessor, const pg::ResourceMapSingletonComponent& resourcesComponent, std::vector<DrawItem>& worldItems, std::vector<DrawItem>& uiItems)
	{
		auto viewDrawQuad = accessor.View<const pg::DrawQuadInFrameEvent>();
		for (auto ent : viewDrawQuad)
		{
			const pg::DrawQuadInFrameEvent& event = viewDrawQuad.get<const pg::DrawQuadInFrameEvent>(ent);
			const pg::UUID textureID = event.m_TextureID.IsNull() ? resourcesComponent.m_DefaultTexture : event.m_TextureID;
			const glm::vec4 color = event.m_TextureID.IsNull() ? glm::vec4(event.m_Color, 1.f) : glm::vec4(1.f);
			// World draws are Y-up: flip the texture V so they sample upright on DirectX (V-down).
			AppendQuad(worldItems, resourcesComponent, event.m_Transform, color, textureID, glm::vec4(0.f, 0.f, 1.f, 1.f), event.m_Origin, event.m_SortKey, glm::vec4(0.f), pg::Texture2D::FlipY());
		}

		auto viewDrawSprite = accessor.View<const pg::DrawSpriteInFrameEvent>();
		for (auto ent : viewDrawSprite)
		{
			const pg::DrawSpriteInFrameEvent& event = viewDrawSprite.get<const pg::DrawSpriteInFrameEvent>(ent);
			const pg::Sprite& sprite = event.m_Sprite;
			AppendQuad(worldItems, resourcesComponent, sprite.GetTransform(), glm::vec4(1.f), sprite.GetTextureID(), sprite.GetTexCoordsRect(), sprite.GetOrigin(), event.m_SortKey, glm::vec4(0.f), pg::Texture2D::FlipY());
		}

		auto viewDrawString = accessor.View<const pg::DrawStringInFrameEvent>();
		for (auto ent : viewDrawString)
		{
			const pg::DrawStringInFrameEvent& event = viewDrawString.get<const pg::DrawStringInFrameEvent>(ent);
			// World text is reflected about its anchor so it reads top-to-bottom on the Y-up screen.
			AppendString(worldItems, resourcesComponent, event.m_Transform, event.m_String, event.m_Font, event.m_Color, event.m_Kerning, event.m_Linespacing, event.m_SortKey, glm::vec4(0.f), -1, pg::Texture2D::FlipY());
		}

		// UI elements keep their nesting level packed into the transform z; it is used purely to order
		// the UI pass among itself. UI is never part of the world Y-sort.
		auto viewDrawUIQuad = accessor.View<const pg::DrawUIQuadInFrameEvent>();
		for (auto ent : viewDrawUIQuad)
		{
			const pg::DrawUIQuadInFrameEvent& event = viewDrawUIQuad.get<const pg::DrawUIQuadInFrameEvent>(ent);
			const pg::UUID textureID = event.m_TextureID.IsNull() ? resourcesComponent.m_DefaultTexture : event.m_TextureID;
			const glm::vec4 color = event.m_TextureID.IsNull() ? glm::vec4(event.m_Color, 1.f) : glm::vec4(1.f);
			// UI is authored in a y-down canvas placed by the inverted UI camera; no texture V flip.
			AppendQuad(uiItems, resourcesComponent, event.m_Transform, color, textureID, event.m_TexCoords, event.m_Origin, event.m_Transform[3][2], event.m_ClipRect, false);
		}

		auto viewDrawUIString = accessor.View<const pg::DrawUIStringInFrameEvent>();
		for (auto ent : viewDrawUIString)
		{
			const pg::DrawUIStringInFrameEvent& event = viewDrawUIString.get<const pg::DrawUIStringInFrameEvent>(ent);
			PG_CORE_EXCEPT(resourcesComponent.m_FontMap.find(event.m_FontID) != resourcesComponent.m_FontMap.end(), "Could not find font");
			AppendString(uiItems, resourcesComponent, event.m_Transform, event.m_String, resourcesComponent.m_FontMap.at(event.m_FontID), event.m_Color, event.m_Kerning, event.m_Linespacing, event.m_Transform[3][2], event.m_ClipRect, event.m_VisibleChars, false);
		}

		// World draws are Y-sorted for a Y-up world: higher Y (further back) draws first, so lower-Y
		// geometry (closer to the camera bottom) draws in front. UI keeps ascending order: its sort key
		// is the packed nesting depth, where a higher level must draw on top.
		std::stable_sort(worldItems.begin(), worldItems.end(),
			[](const DrawItem& lhs, const DrawItem& rhs) { return lhs.m_SortKey > rhs.m_SortKey; });
		std::stable_sort(uiItems.begin(), uiItems.end(),
			[](const DrawItem& lhs, const DrawItem& rhs) { return lhs.m_SortKey < rhs.m_SortKey; });
	}

	// Rebinds the per-frame view-projection to a new camera without re-clearing the target. The matrix
	// lives in a device-level constant buffer shared by the quad and text shaders, so a single upload
	// switches the camera for both. Used to swap from the world camera to the UI camera mid-frame.
	void UploadCamera(pg::RendererDataSingletonComponent& rendererDataComponent, const pg::OrthographicCamera& camera)
	{
		rendererDataComponent.m_QuadShader->Bind();
		rendererDataComponent.m_QuadShader->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Render(pg::CheckedRegistryAccessor& accessor, const pg::OrthographicCamera& ortoCamera, const pg::OrthographicCamera* uiCamera, pg::RendererDataSingletonComponent& rendererDataComponent, const pg::ResourceMapSingletonComponent& resourcesComponent)
	{
		std::vector<DrawItem> worldItems;
		std::vector<DrawItem> uiItems;
		CollectDrawItems(accessor, resourcesComponent, worldItems, uiItems);

		Clear({ 0.3f, 0.3f, 0.3f, 1.f });

		// World pass: the gameplay camera (pan/zoom/aspect). BeginScene clears the target, so it must run
		// before the UI pass.
		BeginScene(ortoCamera, rendererDataComponent, resourcesComponent);
		DrawOrderedItems(rendererDataComponent, resourcesComponent, worldItems, false);

		// UI pass: a dedicated screen-space camera, drawn after the world so UI is always on top. The UI
		// camera appears one frame after the UI config (both created by UIRenderSystem); until then UI is
		// not drawn.
		if (uiCamera != nullptr)
		{
			UploadCamera(rendererDataComponent, *uiCamera);
			DrawOrderedItems(rendererDataComponent, resourcesComponent, uiItems, true);
		}

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
		std::type_index(typeid(pg::UICameraSingletonComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::DrawQuadInFrameEvent)),
		std::type_index(typeid(pg::DrawSpriteInFrameEvent)),
		std::type_index(typeid(pg::DrawStringInFrameEvent)),
		std::type_index(typeid(pg::DrawUIQuadInFrameEvent)),
		std::type_index(typeid(pg::DrawUIStringInFrameEvent)),
		// Ordering-only dependency: forces the 3D pass (which writes this) to run first, so the 2D pass
		// samples the current frame's offscreen image and rebinds the window back buffer last (ADR 0007).
		std::type_index(typeid(pg::Renderer3DDataSingletonComponent)),
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

	// The screen-space UI camera is owned by the UI module; it may be absent for the first frame(s),
	// in which case the UI pass is skipped.
	auto viewUICamera = accessor.View<const pg::UICameraSingletonComponent>();
	const pg::OrthographicCamera* uiCamera = viewUICamera.size() == 1
		? &viewUICamera.get<const pg::UICameraSingletonComponent>(viewUICamera.front()).m_Camera
		: nullptr;

	if (rendererDataView.empty())
	{
		pg::RendererDataSingletonComponent rendererDataComponent;
		pg::ecs::Entity singletonEntity = accessor.Create();
		Init(rendererDataComponent, resourcesComponent, configComponent);
		Render(accessor, ortoCamera, uiCamera, rendererDataComponent, resourcesComponent);
		accessor.EmplaceDeferred<pg::RendererDataSingletonComponent>(singletonEntity, std::move(rendererDataComponent));
	}
	else
	{
		pg::RendererDataSingletonComponent& rendererDataComponent = rendererDataView.get<pg::RendererDataSingletonComponent>(rendererDataView.front());
		Render(accessor, ortoCamera, uiCamera, rendererDataComponent, resourcesComponent);
	}
}


