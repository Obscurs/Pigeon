#pragma once

#include "Pigeon/ECS/World.h"

#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/OrthographicCamera.h"

namespace pig::ui
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

	struct BaseComponent
	{
		BaseComponent() = default;
		BaseComponent(const BaseComponent&) = default;

		glm::vec2 m_Size{};

		glm::vec2 m_Spacing{};

		EHAlignType m_HAlign{ EHAlignType::eLeft };
		EVAlignType m_VAlign{ EVAlignType::eTop };

		entt::entity m_Parent{ entt::null };
		pig::UUID m_UUID{};
		bool m_Enabled{ true };
	};

	struct TextComponent
	{
		TextComponent() = default;
		TextComponent(const TextComponent&) = default;

		std::string m_Text{ "" };
		glm::vec4 m_Color{ 0.f, 0.f, 0.f, 1.f };
		float m_Kerning{ 1.f };
		float m_Spacing{ 1.f };
	};

	struct ImageComponent
	{
		ImageComponent() = default;
		ImageComponent(const ImageComponent&) = default;

		pig::UUID m_TextureHandle;
	};

	struct LoadLayoutOneFrameComponent
	{
		LoadLayoutOneFrameComponent() = default;
		
		std::string m_LayoutFilePath;
	};

	struct RendererConfig
	{
		RendererConfig() = default;
		RendererConfig(const RendererConfig&) = default;

		//ARNAU TODO store fonts in some kind of resource manager and hold just a uuid here
		pig::S_Ptr<pig::Font> m_Font;

		//ARNAU TODO do not use hardcoded orto values
		pig::OrthographicCamera m_Camera{0.f, 1280.0f, -720.0f, 0 };

		float m_Width{ 1920.f };
		float m_Height{ 1080.f };
	};

	struct UIUpdateTransformOneFrameComponent
	{
		UIUpdateTransformOneFrameComponent() = default;
		UIUpdateTransformOneFrameComponent(const UIUpdateTransformOneFrameComponent&) = default;

		glm::vec2 m_Size{};

		glm::vec2 m_Spacing{};

		EHAlignType m_HAlign{ EHAlignType::eLeft };
		EVAlignType m_VAlign{ EVAlignType::eTop };
	};

	struct UIUpdateParentOneFrameComponent
	{
		UIUpdateParentOneFrameComponent() = default;
		UIUpdateParentOneFrameComponent(const UIUpdateParentOneFrameComponent&) = default;

		entt::entity m_Parent{ entt::null };
	};

	struct UIUpdateUUIDOneFrameComponent
	{
		UIUpdateUUIDOneFrameComponent() = default;
		UIUpdateUUIDOneFrameComponent(const UIUpdateUUIDOneFrameComponent&) = default;

		pig::UUID m_UUID{};
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

		bool m_DUMMYVAR{};
	};

	struct UIUpdateImageUUIDOneFrameComponent
	{
		UIUpdateImageUUIDOneFrameComponent() = default;
		UIUpdateImageUUIDOneFrameComponent(const UIUpdateImageUUIDOneFrameComponent&) = default;

		pig::UUID m_UUID{};

		//ARNAU TODO use this to destroy previous images in UI render system? or rethink 
		pig::UUID m_PreviousImageToDestroy{};
	};

	struct UIUpdateTextOneFrameComponent
	{
		UIUpdateTextOneFrameComponent() = default;
		UIUpdateTextOneFrameComponent(const UIUpdateTextOneFrameComponent&) = default;

		std::string m_Text{};
		glm::vec4 m_Color{};
		float m_Spacing{};
		float m_Kerning{};
	};

	//ARNAU TODO rethink, maybe we do not need OFC for this
	struct UIOnClickOneFrameComponent
	{
		UIOnClickOneFrameComponent() = default;
		UIOnClickOneFrameComponent(const UIOnClickOneFrameComponent&) = default;

		pig::UUID m_ElementID;
	};

	struct UIOnHoverOneFrameComponent
	{
		UIOnHoverOneFrameComponent() = default;
		UIOnHoverOneFrameComponent(const UIOnHoverOneFrameComponent&) = default;

		pig::UUID m_ElementID;
	};

	struct UIOnReleaseOneFrameComponent
	{
		UIOnReleaseOneFrameComponent() = default;
		UIOnReleaseOneFrameComponent(const UIOnReleaseOneFrameComponent&) = default;

		pig::UUID m_ElementID;
	};
}
