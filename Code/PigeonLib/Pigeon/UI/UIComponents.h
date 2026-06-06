#pragma once

#include "Pigeon/ECS/World.h"

#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/OrthographicCamera.h"

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

	struct BaseComponent
	{
		BaseComponent() = default;
		BaseComponent(const BaseComponent&) = default;

		glm::vec2 m_Size{};

		glm::vec2 m_Spacing{};

		EHAlignType m_HAlign{ EHAlignType::eLeft };
		EVAlignType m_VAlign{ EVAlignType::eTop };

		pg::ecs::Entity m_Parent{ pg::ecs::null };
		pg::UUID m_UUID{};
		bool m_Enabled{ true };
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
	};

	struct ImageComponent
	{
		ImageComponent() = default;
		ImageComponent(const ImageComponent&) = default;

		pg::UUID m_TextureHandle;
	};

	struct LoadLayoutEvent
	{
		LoadLayoutEvent() = default;
		
		pg::UUID m_UUID;
	};

	struct RendererConfigSingletonComponent
	{
		RendererConfigSingletonComponent() = default;
		RendererConfigSingletonComponent(const RendererConfigSingletonComponent&) = default;

		pg::OrthographicCamera m_Camera{0.f, 1280.0f, -720.0f, 0 };

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
}
