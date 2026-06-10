#pragma once

#include "Pigeon/ECS/World.h"

#include "Pigeon/Renderer/Font.h"

namespace pg::ui
{
	enum class EHAlignType
	{
		eCenter,
		eLeft,
		eRight
	};
	enum class EVAlignType
	{
		eCenter,
		eTop,
		eBottom
	};

	enum class ELayoutType
	{
		eVertical,
		eHorizontal,
		eGrid
	};

	// Anchor-rect layout. Anchors are normalized 0..1 points in the parent rect; pivot is the
	// element's own 0..1 reference point. m_AnchoredPosition places the pivot relative to the anchor;
	// m_Size is the literal pixel size when point-anchored (anchorMin == anchorMax), or a delta added to
	// the stretched span when anchorMin != anchorMax (0 = exactly fills the span). Defaults to a top-left
	// point anchor.
	struct BaseComponent
	{
		BaseComponent() = default;
		BaseComponent(const BaseComponent&) = default;

		glm::vec2 m_AnchorMin{ 0.f, 0.f };
		glm::vec2 m_AnchorMax{ 0.f, 0.f };
		glm::vec2 m_Pivot{ 0.f, 0.f };
		glm::vec2 m_AnchoredPosition{ 0.f, 0.f };
		glm::vec2 m_Size{ 0.f, 0.f };

		pg::ecs::Entity m_Parent{ pg::ecs::null };
		pg::UUID m_UUID{};
		bool m_Enabled{ true };

		// Deterministic order among siblings (authoring order), used only when the parent is a layout
		// container; ignored otherwise.
		int m_SiblingIndex{ 0 };
	};

	// Marks an element as a clip region: its descendants are masked (scissored) to the element's
	// rect, and m_ScrollOffset translates the clipped content (so an app can scroll it). Presence of this
	// component enables clipping; absence means no clipping.
	struct UIClipComponent
	{
		UIClipComponent() = default;
		UIClipComponent(const UIClipComponent&) = default;

		glm::vec2 m_ScrollOffset{ 0.f, 0.f };
	};

	// A layout placed on a parent element: positions its direct children automatically instead of each
	// child using its own anchor rect. Stacks place children along an axis stretched to fill
	// the cross-axis; a grid wraps fixed-size cells after m_Columns columns. Padding is left/top/right/
	// bottom; spacing is the inter-item gap (x between columns, y between rows).
	struct LayoutContainerComponent
	{
		LayoutContainerComponent() = default;
		LayoutContainerComponent(const LayoutContainerComponent&) = default;

		ELayoutType m_Type{ ELayoutType::eVertical };
		glm::vec4 m_Padding{ 0.f, 0.f, 0.f, 0.f };
		glm::vec2 m_Spacing{ 0.f, 0.f };

		int m_Columns{ 1 };
		glm::vec2 m_CellSize{ 0.f, 0.f };
	};

	struct TextComponent
	{
		TextComponent() = default;
		TextComponent(const TextComponent&) = default;

		pg::UUID m_FontID{};
		std::string m_Text{ "" };
		glm::vec4 m_Color{ 0.f, 0.f, 0.f, 1.f };
		float m_Kerning{ 1.f };
		float m_Spacing{ 1.f };

		// Glyph-block alignment within the element's rect, independent of the rect's own anchors.
		EHAlignType m_HAlign{ EHAlignType::eCenter };
		EVAlignType m_VAlign{ EVAlignType::eCenter };
	};

	struct ImageComponent
	{
		ImageComponent() = default;
		ImageComponent(const ImageComponent&) = default;

		pg::UUID m_TextureHandle;

		// Nine-slice border insets (left, top, right, bottom) in texture pixels. When any is non-zero the
		// image is drawn as a 3x3 grid so the corners keep their size while edges/center stretch.
		// All-zero draws the texture as a single stretched quad.
		glm::vec4 m_NineSliceBorder{ 0.f, 0.f, 0.f, 0.f };
	};

	struct LoadLayoutEvent
	{
		LoadLayoutEvent() = default;
		
		pg::UUID m_UUID;
	};

	// Live UI canvas state. Owned by UIRenderSystem, read by UIEventSystem and the layout
	// helpers. UI is authored against the reference canvas; the scale factor and logical canvas are
	// derived each frame from the live window size so layout fills the screen and hit-testing matches
	// rendering at any window resolution.
	struct RendererConfigSingletonComponent
	{
		RendererConfigSingletonComponent() = default;
		RendererConfigSingletonComponent(const RendererConfigSingletonComponent&) = default;

		// Reference (authoring) canvas + Unity-style match factor (0 = match width, 1 = match height);
		// seeded from EngineConfigSingletonComponent.
		float m_RefWidth{ 1920.f };
		float m_RefHeight{ 1080.f };
		float m_MatchFactor{ 0.5f };

		// Live window client size the scale was last derived from (0 until first seeded).
		float m_WinWidth{ 0.f };
		float m_WinHeight{ 0.f };

		// Derived from window-vs-reference: the uniform reference->window-pixel scale, and the live
		// logical canvas (the window size expressed in reference units) that layout resolves against.
		float m_ScaleFactor{ 1.f };
		float m_Width{ 1920.f };
		float m_Height{ 1080.f };
	};

	// Mirrors the BaseComponent anchor-rect fields; UIControlSystem copies them onto the element's
	// BaseComponent when present.
	struct UIUpdateTransformOneFrameComponent
	{
		UIUpdateTransformOneFrameComponent() = default;
		UIUpdateTransformOneFrameComponent(const UIUpdateTransformOneFrameComponent&) = default;

		glm::vec2 m_AnchorMin{ 0.f, 0.f };
		glm::vec2 m_AnchorMax{ 0.f, 0.f };
		glm::vec2 m_Pivot{ 0.f, 0.f };
		glm::vec2 m_AnchoredPosition{ 0.f, 0.f };
		glm::vec2 m_Size{ 0.f, 0.f };
	};

	struct UIUpdateParentOneFrameComponent
	{
		UIUpdateParentOneFrameComponent() = default;
		UIUpdateParentOneFrameComponent(const UIUpdateParentOneFrameComponent&) = default;

		pg::ecs::Entity m_Parent{ pg::ecs::null };
	};

	struct UIUpdateUUIDOneFrameComponent
	{
		UIUpdateUUIDOneFrameComponent() = default;
		UIUpdateUUIDOneFrameComponent(const UIUpdateUUIDOneFrameComponent&) = default;

		pg::UUID m_UUID{};
	};

	struct UIUpdateEnableOneFrameComponent
	{
		UIUpdateEnableOneFrameComponent() = default;
		UIUpdateEnableOneFrameComponent(const UIUpdateEnableOneFrameComponent&) = default;

		bool m_Enabled{};
	};

	struct UIDestroyOneFrameComponent
	{
		UIDestroyOneFrameComponent() = default;
		UIDestroyOneFrameComponent(const UIDestroyOneFrameComponent&) = default;

		bool m_Dummy{};
	};

	// Runtime mutation of a clip element's scroll offset (drives scrolling); UIControlSystem copies it
	// onto the element's UIClipComponent.
	struct UIUpdateClipOffsetOneFrameComponent
	{
		UIUpdateClipOffsetOneFrameComponent() = default;
		UIUpdateClipOffsetOneFrameComponent(const UIUpdateClipOffsetOneFrameComponent&) = default;

		glm::vec2 m_ScrollOffset{ 0.f, 0.f };
	};

	struct UIUpdateImageUUIDOneFrameComponent
	{
		UIUpdateImageUUIDOneFrameComponent() = default;
		UIUpdateImageUUIDOneFrameComponent(const UIUpdateImageUUIDOneFrameComponent&) = default;

		pg::UUID m_UUID{};
	};

	struct UIUpdateTextOneFrameComponent
	{
		UIUpdateTextOneFrameComponent() = default;
		UIUpdateTextOneFrameComponent(const UIUpdateTextOneFrameComponent&) = default;

		pg::UUID m_FontID{};
		std::string m_Text{};
		glm::vec4 m_Color{};
		float m_Spacing{};
		float m_Kerning{};
	};

	struct UIOnClickOneFrameComponent
	{
		UIOnClickOneFrameComponent() = default;
		UIOnClickOneFrameComponent(const UIOnClickOneFrameComponent&) = default;

		pg::UUID m_ElementID;
	};

	struct UIOnHoverOneFrameComponent
	{
		UIOnHoverOneFrameComponent() = default;
		UIOnHoverOneFrameComponent(const UIOnHoverOneFrameComponent&) = default;

		pg::UUID m_ElementID;
	};

	struct UIOnReleaseOneFrameComponent
	{
		UIOnReleaseOneFrameComponent() = default;
		UIOnReleaseOneFrameComponent(const UIOnReleaseOneFrameComponent&) = default;

		pg::UUID m_ElementID;
	};

	// Raised (in-frame) when the pointer is over an interactive UI element (one carrying an ImageComponent
	// or TextComponent), so gameplay systems can ignore a pointer interaction that landed on UI. Bare
	// layout containers do not raise it. Carries the captured element's UUID.
	struct UIInputCapturedInFrameEvent
	{
		UIInputCapturedInFrameEvent() = default;
		UIInputCapturedInFrameEvent(const UIInputCapturedInFrameEvent&) = default;

		pg::UUID m_ElementID;
	};
}
