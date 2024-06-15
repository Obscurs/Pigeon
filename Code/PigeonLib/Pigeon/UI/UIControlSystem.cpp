#include "UIControlSystem.h"

#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/Renderer2D.h"

namespace
{
	std::string ReadFileToString(const std::string& filePath) 
	{
		std::ifstream file(filePath);
		if (!file.is_open()) {
			PG_CORE_ASSERT(false, "Could not open file");
			return "";
		}

		std::ostringstream ss;
		ss << file.rdbuf();
		file.close();
		return ss.str();
	}
}
pig::ui::UIControlSystem::UIControlSystem(pig::S_Ptr<IUIControlSystemHelper> helper)
	: pig::System(pig::SystemType::eTest)
	, m_Helper(helper)
{
}

void pig::ui::UIControlSystem::Update(float dt)
{
	auto viewTransform = pig::World::GetRegistry().view<pig::ui::BaseComponent, const pig::ui::UIUpdateTransformOneFrameComponent>();
	for (auto ent : viewTransform)
	{
		pig::ui::BaseComponent& baseComponent = viewTransform.get<pig::ui::BaseComponent>(ent);
		const pig::ui::UIUpdateTransformOneFrameComponent& updateComponent = viewTransform.get<const pig::ui::UIUpdateTransformOneFrameComponent>(ent);
		baseComponent.m_HAlign = updateComponent.m_HAlign;
		baseComponent.m_VAlign = updateComponent.m_VAlign;
		baseComponent.m_Size = updateComponent.m_Size;
		baseComponent.m_Spacing = updateComponent.m_Spacing;
	}

	auto viewParent = pig::World::GetRegistry().view<pig::ui::BaseComponent, const pig::ui::UIUpdateParentOneFrameComponent>();
	for (auto ent : viewParent)
	{
		pig::ui::BaseComponent& baseComponent = viewParent.get<pig::ui::BaseComponent>(ent);
		const pig::ui::UIUpdateParentOneFrameComponent& updateComponent = viewParent.get<const pig::ui::UIUpdateParentOneFrameComponent>(ent);
		baseComponent.m_Parent = updateComponent.m_Parent;
	}

	auto viewEnable = pig::World::GetRegistry().view<pig::ui::BaseComponent, const pig::ui::UIUpdateEnableOneFrameComponent>();
	for (auto ent : viewEnable)
	{
		pig::ui::BaseComponent& baseComponent = viewEnable.get<pig::ui::BaseComponent>(ent);
		const pig::ui::UIUpdateEnableOneFrameComponent& updateComponent = viewEnable.get<const pig::ui::UIUpdateEnableOneFrameComponent>(ent);
		baseComponent.m_Enabled = updateComponent.m_Enabled;
	}

	auto viewUUID = pig::World::GetRegistry().view<pig::ui::BaseComponent, const pig::ui::UIUpdateUUIDOneFrameComponent>();
	for (auto ent : viewUUID)
	{
		pig::ui::BaseComponent& baseComponent = viewParent.get<pig::ui::BaseComponent>(ent);
		const pig::ui::UIUpdateUUIDOneFrameComponent& updateComponent = viewUUID.get<const pig::ui::UIUpdateUUIDOneFrameComponent>(ent);
		baseComponent.m_UUID = updateComponent.m_UUID;
	}
	
	auto viewImage = pig::World::GetRegistry().view<pig::ui::ImageComponent, const pig::ui::UIUpdateImageUUIDOneFrameComponent>();
	for (auto ent : viewImage)
	{
		pig::ui::ImageComponent& imageComponent = viewImage.get<pig::ui::ImageComponent>(ent);
		const pig::ui::UIUpdateImageUUIDOneFrameComponent& updateComponent = viewImage.get<const pig::ui::UIUpdateImageUUIDOneFrameComponent>(ent);
		imageComponent.m_TextureHandle = updateComponent.m_UUID;
	}

	auto viewText = pig::World::GetRegistry().view<pig::ui::TextComponent, const pig::ui::UIUpdateTextOneFrameComponent>();
	for (auto ent : viewText)
	{
		pig::ui::TextComponent& textComponent = viewText.get<pig::ui::TextComponent>(ent);
		const pig::ui::UIUpdateTextOneFrameComponent& updateComponent = viewText.get<const pig::ui::UIUpdateTextOneFrameComponent>(ent);
		textComponent.m_Color = updateComponent.m_Color;
		textComponent.m_Kerning = updateComponent.m_Kerning;
		textComponent.m_Spacing = updateComponent.m_Spacing;
		textComponent.m_Text = updateComponent.m_Text;
	}

	auto viewLoad = pig::World::GetRegistry().view<const pig::ui::LoadLayoutOneFrameComponent>();
	for (auto entJson : viewLoad)
	{
		LoadLayoutFromFile(viewLoad.get<const pig::ui::LoadLayoutOneFrameComponent>(entJson).m_LayoutFilePath);
	}

	auto viewDestroy = pig::World::GetRegistry().view<const pig::ui::UIDestroyOneFrameComponent>();
	for (auto ent : viewDestroy)
	{
		DestroyUI(ent);
	}
}

void pig::ui::UIControlSystem::LoadLayoutFromFile(const std::string& path)
{
	const std::string jsonString = ReadFileToString(path);
	json jsonObject = json::parse(jsonString);
	ParseJsonUIElement(jsonObject, entt::null);
}

void pig::ui::UIControlSystem::ParseJsonUIElement(const json& jsonObject, entt::entity parent)
{
	if (jsonObject.contains("ui") && jsonObject["ui"].is_object())
	{
		const json& uiJsonObject = jsonObject["ui"];
		entt::entity ent = pig::World::GetRegistry().create();
		ParseBaseComponentFromJson(uiJsonObject, ent, parent);

		if (uiJsonObject.contains("image") && uiJsonObject["image"].is_object())
		{
			PG_CORE_EXCEPT(!uiJsonObject.contains("text"), "we should have either a image or a text but not both");
			ParseImageComponentFromJson(uiJsonObject["image"], ent);
		}
		if (uiJsonObject.contains("text") && uiJsonObject["text"].is_object())
		{
			PG_CORE_EXCEPT(!uiJsonObject.contains("image"), "we should have either a image or a text but not both");
			ParseTextComponentFromJson(uiJsonObject["text"], ent);
		}
	}
	else if (jsonObject.contains("uiFile") && jsonObject["uiFile"].is_string())
	{
		std::string path = jsonObject["uiFile"].get<std::string>();
		const std::string jsonLoadedString = ReadFileToString(path);
		json jsonLoadedObject = json::parse(jsonLoadedString);
		ParseJsonUIElement(jsonLoadedObject, parent);
	}
}

void pig::ui::UIControlSystem::ParseBaseComponentFromJson(const json& jsonObject, entt::entity ent, entt::entity parent)
{
	pig::ui::BaseComponent& baseComp = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(ent);
	baseComp.m_Parent = parent;

	if (jsonObject.contains("width"))
	{
		PG_CORE_EXCEPT(jsonObject["width"].is_number(), "unable to parse json, width is not a number");
		baseComp.m_Size.x = jsonObject["width"].get<float>();
	}
	if (jsonObject.contains("height"))
	{
		PG_CORE_EXCEPT(jsonObject["height"].is_number(), "unable to parse json, height is not a number");
		baseComp.m_Size.y = jsonObject["height"].get<float>();
	}
	if (jsonObject.contains("spacing_x"))
	{
		PG_CORE_EXCEPT(jsonObject["spacing_x"].is_number(), "unable to parse json, spacing_x is not a number");
		baseComp.m_Spacing.x = jsonObject["spacing_x"].get<float>();
	}
	if (jsonObject.contains("spacing_y"))
	{
		PG_CORE_EXCEPT(jsonObject["spacing_y"].is_number(), "unable to parse json, spacing_y is not a number");
		baseComp.m_Spacing.y = jsonObject["spacing_y"].get<float>();
	}

	if (jsonObject.contains("alignment_h"))
	{
		PG_CORE_EXCEPT(jsonObject["alignment_h"].is_string(), "unable to parse json, alignment_h is not a string");
		std::string alignmentStr = jsonObject["alignment_h"].get<std::string>();
		if (alignmentStr == "left")
		{
			baseComp.m_HAlign = EHAlignType::eLeft;
		}
		else if (alignmentStr == "right")
		{
			baseComp.m_HAlign = EHAlignType::eRight;
		}
		else if (alignmentStr == "center")
		{
			baseComp.m_HAlign = EHAlignType::eCenter;
		}
	}

	if (jsonObject.contains("alignment_v"))
	{
		PG_CORE_EXCEPT(jsonObject["alignment_v"].is_string(), "unable to parse json, alignment_v is not a string");
		std::string alignmentStr = jsonObject["alignment_v"].get<std::string>();
		if (alignmentStr == "top")
		{
			baseComp.m_VAlign = EVAlignType::eTop;
		}
		else if (alignmentStr == "bottom")
		{
			baseComp.m_VAlign = EVAlignType::eBottom;
		}
		else if (alignmentStr == "center")
		{
			baseComp.m_VAlign = EVAlignType::eCenter;
		}
	}

	if (jsonObject.contains("uuid"))
	{
		baseComp.m_UUID = pig::UUID(jsonObject["uuid"].get<std::string>());
	}

	if (jsonObject.contains("children"))
	{
		PG_CORE_EXCEPT(jsonObject["children"].is_array(), "unable to parse json, children is not a array");
		for (json child : jsonObject["children"])
		{
			ParseJsonUIElement(child, ent);
		}
	}
}

void pig::ui::UIControlSystem::ParseImageComponentFromJson(const json& jsonObject, entt::entity ent)
{
	pig::ui::ImageComponent& component = pig::World::GetRegistry().emplace<pig::ui::ImageComponent>(ent);
	if (jsonObject.contains("path"))
	{
		PG_CORE_EXCEPT(jsonObject["path"].is_string(), "unable to parse json, image path is not a string");
		component.m_TextureHandle = m_Helper->CreateUIImageFromPath(jsonObject["path"].get<std::string>());
	}
}

void pig::ui::UIControlSystem::ParseTextComponentFromJson(const json& jsonObject, entt::entity ent)
{
	pig::ui::TextComponent& component = pig::World::GetRegistry().emplace<pig::ui::TextComponent>(ent);
	if (jsonObject.contains("colR"))
	{
		PG_CORE_EXCEPT(jsonObject["colR"].is_number(), "unable to parse json, colR is not a number");
		component.m_Color.r = jsonObject["colR"].get<float>();
	}
	if (jsonObject.contains("colG"))
	{
		PG_CORE_EXCEPT(jsonObject["colG"].is_number(), "unable to parse json, colG is not a number");
		component.m_Color.g = jsonObject["colG"].get<float>();
	}
	if (jsonObject.contains("colB"))
	{
		PG_CORE_EXCEPT(jsonObject["colB"].is_number(), "unable to parse json, colB is not a number");
		component.m_Color.b = jsonObject["colB"].get<float>();
	}
	if (jsonObject.contains("colA"))
	{
		PG_CORE_EXCEPT(jsonObject["colA"].is_number(), "unable to parse json, colA is not a number");
		component.m_Color.a = jsonObject["colA"].get<float>();
	}
	if (jsonObject.contains("kerning"))
	{
		PG_CORE_EXCEPT(jsonObject["kerning"].is_number(), "unable to parse json, kerning is not a number");
		component.m_Kerning = jsonObject["kerning"].get<float>();
	}
	if (jsonObject.contains("spacing"))
	{
		PG_CORE_EXCEPT(jsonObject["spacing"].is_number(), "unable to parse json, spacing is not a number");
		component.m_Spacing = jsonObject["spacing"].get<float>();
	}
	if (jsonObject.contains("string"))
	{
		PG_CORE_EXCEPT(jsonObject["string"].is_string(), "unable to parse json, string is not a string");
		component.m_Text = jsonObject["string"].get<std::string>();
	}
}

void pig::ui::UIControlSystem::DestroyUI(entt::entity ent)
{
	std::vector<entt::entity> children = pig::ui::GetUIChildrenForElement(ent);
	for (entt::entity& child : children)
	{
		DestroyUI(child);
	}
	pig::World::GetRegistry().destroy(ent);
}

pig::UUID pig::ui::UIControlSystemHelper::CreateUIImageFromPath(const std::string& path)
{
	//TODO ARNAU: maybe UI renderer should handle this, or have a image system
	return pig::Renderer2D::AddTexture(path, pig::EMappedTextureType::eQuad);
}
