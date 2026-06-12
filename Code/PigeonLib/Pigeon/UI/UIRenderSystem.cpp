#include "Pigeon/UI/UIRenderSystem.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawUIQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIStringInFrameEvent.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/UICameraSingletonComponent.h"
#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

#include <cmath>

namespace
{
	// Unity-style "match width or height" UI scale factor: blend in log2 space between the
	// window/reference width ratio and the height ratio, then exponentiate. match=0 follows width,
	// match=1 follows height. Equal-aspect windows give the same factor on both axes for any match.
	float ComputeUIScaleFactor(float winW, float winH, float refW, float refH, float match)
	{
		const float logW = std::log2(winW / refW);
		const float logH = std::log2(winH / refH);
		const float logBlend = logW + (logH - logW) * match;
		return std::pow(2.f, logBlend);
	}

	// Refreshes the live UI canvas (scale factor + logical size) from the engine config and window size,
	// and points the screen-space UI camera at the logical canvas. Layout and hit-testing both resolve
	// against the logical canvas so they stay consistent at any window resolution.
	void UpdateUICanvas(pg::CheckedRegistryAccessor& accessor, pg::ui::RendererConfigSingletonComponent& cfg,
		const pg::EngineConfigSingletonComponent& engineConfig)
	{
		cfg.m_RefWidth = engineConfig.m_UIReferenceWidth;
		cfg.m_RefHeight = engineConfig.m_UIReferenceHeight;
		cfg.m_MatchFactor = engineConfig.m_UIMatchFactor;

		// Prefer a resize event raised this frame; otherwise seed the window size from the engine config
		// once (m_WinWidth/Height stay set across later frames so the live size survives resize-less frames).
		auto viewResize = accessor.View<const pg::WindowResizeEventComponent>();
		if (viewResize.size() > 0)
		{
			const pg::WindowResizeEventComponent& resize =
				viewResize.get<const pg::WindowResizeEventComponent>(viewResize.front());
			cfg.m_WinWidth = static_cast<float>(resize.m_Width);
			cfg.m_WinHeight = static_cast<float>(resize.m_Height);
		}
		else if (cfg.m_WinWidth <= 0.f || cfg.m_WinHeight <= 0.f)
		{
			cfg.m_WinWidth = static_cast<float>(engineConfig.m_WindowWidth);
			cfg.m_WinHeight = static_cast<float>(engineConfig.m_WindowHeight);
		}

		if (cfg.m_RefWidth <= 0.f || cfg.m_RefHeight <= 0.f || cfg.m_WinWidth <= 0.f || cfg.m_WinHeight <= 0.f)
			return;

		cfg.m_ScaleFactor = ComputeUIScaleFactor(cfg.m_WinWidth, cfg.m_WinHeight, cfg.m_RefWidth, cfg.m_RefHeight, cfg.m_MatchFactor);
		cfg.m_Width = cfg.m_WinWidth / cfg.m_ScaleFactor;
		cfg.m_Height = cfg.m_WinHeight / cfg.m_ScaleFactor;

		auto viewUICam = accessor.View<pg::UICameraSingletonComponent>();
		if (viewUICam.size() == 1)
		{
			pg::UICameraSingletonComponent& uiCam =
				viewUICam.get<pg::UICameraSingletonComponent>(viewUICam.front());
			// Inverted ortho (bottom = height, top = 0) so the y-down canvas renders y=0 at the top.
			const glm::vec2 verticalBounds = pg::ui::GetUICameraOrthoBottomTop(cfg.m_Height);
			uiCam.m_Camera.SetProjection(0.f, cfg.m_Width, verticalBounds.x, verticalBounds.y);
		}
	}

	float AlignOffsetH(pg::ui::EHAlignType align, float boxSize, float contentSize)
	{
		switch (align)
		{
		case pg::ui::EHAlignType::eLeft:  return 0.f;
		case pg::ui::EHAlignType::eRight: return boxSize - contentSize;
		default:                          return (boxSize - contentSize) * 0.5f;
		}
	}

	float AlignOffsetV(pg::ui::EVAlignType align, float boxSize, float contentSize)
	{
		switch (align)
		{
		case pg::ui::EVAlignType::eTop:    return 0.f;
		case pg::ui::EVAlignType::eBottom: return boxSize - contentSize;
		default:                           return (boxSize - contentSize) * 0.5f;
		}
	}

	// Builds the draw transform for a UI element: positions at (posX, posY) in logical-canvas units with
	// the nesting depth packed as Z (for UI draw-layer ordering) and the given uniform scale applied.
	glm::mat4 BuildUITransform(float posX, float posY, int depth, const glm::vec2& scale)
	{
		glm::mat4 transform(1.f);
		transform = glm::translate(transform, glm::vec3(posX, posY, static_cast<float>(depth)));
		transform = glm::scale(transform, glm::vec3(scale, 1.f));
		return transform;
	}

	// Largest uniform font scale that fits the rendered string (measured in font units) inside the box.
	float GetFontSizeForBox(const glm::vec2& boxSize, const glm::vec2& stringBounds, unsigned int numLines)
	{
		const float aspectBox = boxSize.x / boxSize.y;
		const float aspectString = stringBounds.x / stringBounds.y;
		if (aspectBox > aspectString)
		{
			return boxSize.y / (stringBounds.y * numLines);
		}
		return boxSize.x / stringBounds.x;
	}

	// The element's effective clip rect converted from logical-canvas units to window pixels (the scissor
	// space the renderer expects), using the same scale factor that maps the canvas to the window.
	glm::vec4 ComputeClipRectPixels(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent)
	{
		const glm::vec4 clipCanvas = pg::ui::GetElementClipRect(accessor, baseComponent, renderComponent);
		return clipCanvas * renderComponent.m_ScaleFactor;
	}

	void EmitUIQuad(pg::CheckedRegistryAccessor& accessor, const glm::mat4& transform, const pg::UUID& textureID, const glm::vec4& texCoords, const glm::vec4& clipRect)
	{
		pg::DrawUIQuadInFrameEvent quadEvent;
		quadEvent.m_Transform = transform;
		quadEvent.m_TextureID = textureID;
		quadEvent.m_Origin = { 0.f, 0.f, 0.f };
		quadEvent.m_TexCoords = texCoords;
		quadEvent.m_ClipRect = clipRect;
		accessor.EmplaceInframeEvent<pg::DrawUIQuadInFrameEvent>(std::move(quadEvent));
	}

	// Emits the nine cells of a nine-slice image: corners keep the texture-pixel border size while edges
	// and center stretch to fill the rect; each cell samples its matching UV sub-rect.
	void EmitNineSlice(pg::CheckedRegistryAccessor& accessor, const glm::vec4& rect, int depth, const pg::UUID& textureID, const glm::vec4& border, float texWidth, float texHeight, const glm::vec4& clipRect)
	{
		const float xCut[4] = { rect.x, rect.x + border.x, rect.x + rect.z - border.z, rect.x + rect.z };
		const float yCut[4] = { rect.y, rect.y + border.y, rect.y + rect.w - border.w, rect.y + rect.w };
		const float uCut[4] = { 0.f, border.x / texWidth, 1.f - border.z / texWidth, 1.f };
		const float vCut[4] = { 0.f, border.y / texHeight, 1.f - border.w / texHeight, 1.f };

		for (int j = 0; j < 3; ++j)
		{
			for (int i = 0; i < 3; ++i)
			{
				const glm::mat4 transform = BuildUITransform(xCut[i], yCut[j], depth, glm::vec2(xCut[i + 1] - xCut[i], yCut[j + 1] - yCut[j]));
				EmitUIQuad(accessor, transform, textureID, glm::vec4(uCut[i], vCut[j], uCut[i + 1], vCut[j + 1]), clipRect);
			}
		}
	}

	// Draws an image element: a nine-slice grid when it declares a non-zero border and its texture (with
	// valid dimensions) is loaded, otherwise a single stretched quad filling the rect.
	void EmitImage(pg::CheckedRegistryAccessor& accessor, const pg::ResourceMapSingletonComponent& resourcesComponent, const glm::vec4& rect, int depth, const pg::ui::ImageComponent& imageComponent, const glm::vec4& clipRect)
	{
		const glm::vec4& border = imageComponent.m_NineSliceBorder;
		const bool wantsNineSlice = border.x != 0.f || border.y != 0.f || border.z != 0.f || border.w != 0.f;
		if (wantsNineSlice)
		{
			const std::unordered_map<pg::UUID, pg::MappedTexture>::const_iterator it = resourcesComponent.m_TextureMap.find(imageComponent.m_TextureHandle);
			if (it != resourcesComponent.m_TextureMap.end() && it->second.m_Texture && it->second.m_Texture->GetWidth() > 0 && it->second.m_Texture->GetHeight() > 0)
			{
				EmitNineSlice(accessor, rect, depth, imageComponent.m_TextureHandle, border,
					static_cast<float>(it->second.m_Texture->GetWidth()), static_cast<float>(it->second.m_Texture->GetHeight()), clipRect);
				return;
			}
		}
		EmitUIQuad(accessor, BuildUITransform(rect.x, rect.y, depth, glm::vec2(rect.z, rect.w)), imageComponent.m_TextureHandle, glm::vec4(0.f, 0.f, 1.f, 1.f), clipRect);
	}

	glm::vec2 GetStringBounds(const std::string& string, float kerning, float linespace, pg::S_Ptr<pg::Font> font)
	{
		return font->GetStringBounds(string, kerning, linespace);
	}

	unsigned int GetStringNumLines(const std::string& string, pg::S_Ptr<pg::Font> font)
	{
		if (string.empty())
			return 0;

		unsigned int numLines = 1;
		for (size_t i = 0; i < string.size(); i++)
		{
			if (font->IsCharacterNewLine(string[i]))
				++numLines;
		}
		return numLines;
	}
}

pg::SystemAccessDecl pg::ui::UIRenderSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::ImageComponent)),
		std::type_index(typeid(pg::ui::TextComponent)),
		std::type_index(typeid(pg::ui::LayoutContainerComponent)),
		std::type_index(typeid(pg::ui::UIClipComponent)),
		std::type_index(typeid(pg::ui::RendererConfigSingletonComponent)),
		std::type_index(typeid(pg::WindowResizeEventComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::ui::RendererConfigSingletonComponent)),
		std::type_index(typeid(pg::UICameraSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::RendererConfigSingletonComponent)),
		std::type_index(typeid(pg::UICameraSingletonComponent)),
	};
	decl.inframeAddSet = {
		std::type_index(typeid(pg::DrawUIQuadInFrameEvent)),
		std::type_index(typeid(pg::DrawUIStringInFrameEvent)),
	};
	return decl;
}

void pg::ui::UIRenderSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto viewRenderConfig = accessor.View<pg::ui::RendererConfigSingletonComponent>();
	if (viewRenderConfig.size() == 0)
	{
		pg::ui::RendererConfigSingletonComponent config;
		pg::ecs::Entity configEntity = accessor.Create();
		accessor.EmplaceDeferred<pg::ui::RendererConfigSingletonComponent>(configEntity, std::move(config));

		// The renderer's screen-space UI camera is created alongside the canvas config and kept in sync
		// with the logical canvas each frame. It is a pg:: type so the renderer never depends on pg::ui.
		pg::UICameraSingletonComponent uiCamera;
		pg::ecs::Entity uiCameraEntity = accessor.Create();
		accessor.EmplaceDeferred<pg::UICameraSingletonComponent>(uiCameraEntity, std::move(uiCamera));
		return;
	}
	PG_CORE_ASSERT(viewRenderConfig.size() == 1, "There should only be one ui render config component");

	auto viewEngineConfig = accessor.View<const pg::EngineConfigSingletonComponent>();
	auto viewResources = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (viewEngineConfig.empty() || viewResources.empty())
	{
		return;
	}
	const pg::ResourceMapSingletonComponent& resourcesComponent = viewResources.get<const pg::ResourceMapSingletonComponent>(viewResources.front());
	const pg::EngineConfigSingletonComponent& engineConfigComponent = viewEngineConfig.get<const pg::EngineConfigSingletonComponent>(viewEngineConfig.front());
	pg::ui::RendererConfigSingletonComponent& renderComponent = viewRenderConfig.get<pg::ui::RendererConfigSingletonComponent>(viewRenderConfig.front());

	// Derive the live UI canvas + screen-space UI camera before laying out elements this frame.
	UpdateUICanvas(accessor, renderComponent, engineConfigComponent);

	auto viewImages = accessor.View<const pg::ui::BaseComponent, const pg::ui::ImageComponent>();
	for (auto ent : viewImages)
	{
		const pg::ui::BaseComponent& baseComponent = viewImages.get<const pg::ui::BaseComponent>(ent);
		if (pg::ui::IsUIElementEnabled(accessor, baseComponent))
		{
			const pg::ui::ImageComponent& imageComponent = viewImages.get<const pg::ui::ImageComponent>(ent);

			const glm::vec4 rect = pg::ui::GetElementRect(accessor, baseComponent, renderComponent);
			const int depth = pg::ui::GetElementDepth(accessor, baseComponent);
			const glm::vec4 clipRect = ComputeClipRectPixels(accessor, baseComponent, renderComponent);

			EmitImage(accessor, resourcesComponent, rect, depth, imageComponent, clipRect);
		}
	}

	auto viewText = accessor.View<const pg::ui::BaseComponent, const pg::ui::TextComponent>();
	for (auto ent : viewText)
	{
		const pg::ui::BaseComponent& baseComponent = viewText.get<pg::ui::BaseComponent>(ent);
		if (pg::ui::IsUIElementEnabled(accessor, baseComponent))
		{
			const pg::ui::TextComponent& textComponent = viewText.get<pg::ui::TextComponent>(ent);
			const pg::UUID fontID = textComponent.m_FontID.IsNull() ? engineConfigComponent.m_DefaultFontID : textComponent.m_FontID;
			PG_CORE_EXCEPT(resourcesComponent.m_FontMap.find(fontID) != resourcesComponent.m_FontMap.end(), "could not find font for ui text");
			const pg::S_Ptr<pg::Font> font = resourcesComponent.m_FontMap.at(fontID);

			const glm::vec4 rect = pg::ui::GetElementRect(accessor, baseComponent, renderComponent);
			const int depth = pg::ui::GetElementDepth(accessor, baseComponent);

			// Fixed-size body text renders at its declared em height (and may word-wrap to the rect width);
			// otherwise the legacy path auto-fits the whole string to the rect (suited to labels).
			const bool fixedSize = textComponent.m_FixedFontSize > 0.f;
			std::string renderString = textComponent.m_Text;
			if (fixedSize && textComponent.m_WordWrap)
			{
				renderString = font->WrapString(textComponent.m_Text, textComponent.m_FixedFontSize, textComponent.m_Kerning, textComponent.m_Spacing, rect.z);
			}

			const unsigned int numLines = GetStringNumLines(renderString, font);
			const glm::vec2 stringBounds = GetStringBounds(renderString, textComponent.m_Kerning, textComponent.m_Spacing, font);
			const float fontSize = fixedSize ? textComponent.m_FixedFontSize : GetFontSizeForBox(glm::vec2(rect.z, rect.w), stringBounds, numLines);

			// Position the rendered glyph block within the element rect per the text's own alignment.
			const glm::vec2 renderedSize(stringBounds.x * fontSize, stringBounds.y * fontSize * numLines);
			const float posX = rect.x + AlignOffsetH(textComponent.m_HAlign, rect.z, renderedSize.x);
			const float posY = rect.y + AlignOffsetV(textComponent.m_VAlign, rect.w, renderedSize.y);
			const glm::mat4 transform = BuildUITransform(posX, posY, depth, glm::vec2(fontSize, fontSize));

			pg::DrawUIStringInFrameEvent stringEvent;
			stringEvent.m_Transform = transform;
			stringEvent.m_String = renderString;
			stringEvent.m_FontID = fontID;
			stringEvent.m_Color = textComponent.m_Color;
			stringEvent.m_Kerning = textComponent.m_Kerning;
			stringEvent.m_Linespacing = textComponent.m_Spacing;
			stringEvent.m_VisibleChars = textComponent.m_VisibleChars;
			stringEvent.m_ClipRect = ComputeClipRectPixels(accessor, baseComponent, renderComponent);
			accessor.EmplaceInframeEvent<pg::DrawUIStringInFrameEvent>(std::move(stringEvent));
		}
	}
}
