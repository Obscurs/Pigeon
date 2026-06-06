#include "Pigeon/UI/UIControlSystem.h"

#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/CheckedRegistryAccessor.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

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

	void DestroyUI(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent)
	{
		std::vector<pg::ecs::Entity> children = pg::ui::GetUIChildrenForElement(accessor, ent);
		for (pg::ecs::Entity& child : children)
		{
			DestroyUI(accessor, child);
		}
		accessor.destroy_deferred(ent);
	}

	void ParseImageComponentFromJson(pg::CheckedRegistryAccessor& accessor, const json& jsonObject, pg::ecs::Entity ent)
	{
		pg::ui::ImageComponent component;
		if (jsonObject.contains("id"))
		{
			PG_CORE_EXCEPT(jsonObject["id"].is_string(), "unable to parse json, image id is not a string");
			component.m_TextureHandle = pg::UUID(jsonObject["id"].get<std::string>());
		}

		accessor.emplace_deferred<pg::ui::ImageComponent>(ent, std::move(component));
	}

	void ParseTextComponentFromJson(pg::CheckedRegistryAccessor& accessor, const json& jsonObject, pg::ecs::Entity ent)
	{
		pg::ui::TextComponent component;
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
		if (jsonObject.contains("font"))
		{
			PG_CORE_EXCEPT(jsonObject["font"].is_string(), "unable to parse json, font is not a string");
			component.m_FontID = pg::UUID(jsonObject["font"].get<std::string>());
		}
		accessor.emplace_deferred<pg::ui::TextComponent>(ent, std::move(component));
	}

	void ParseJsonUIElement(pg::CheckedRegistryAccessor& accessor, const json& jsonObject, pg::ecs::Entity parent)
	{
		if (jsonObject.contains("ui") && jsonObject["ui"].is_object())
		{
			const json& uiJsonObject = jsonObject["ui"];
			pg::ecs::Entity ent = accessor.create();
			pg::ui::BaseComponent baseComp;
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
					baseComp.m_HAlign = pg::ui::EHAlignType::eLeft;
				}
				else if (alignmentStr == "right")
				{
					baseComp.m_HAlign = pg::ui::EHAlignType::eRight;
				}
				else if (alignmentStr == "center")
				{
					baseComp.m_HAlign = pg::ui::EHAlignType::eCenter;
				}
			}

			if (jsonObject.contains("alignment_v"))
			{
				PG_CORE_EXCEPT(jsonObject["alignment_v"].is_string(), "unable to parse json, alignment_v is not a string");
				std::string alignmentStr = jsonObject["alignment_v"].get<std::string>();
				if (alignmentStr == "top")
				{
					baseComp.m_VAlign = pg::ui::EVAlignType::eTop;
				}
				else if (alignmentStr == "bottom")
				{
					baseComp.m_VAlign = pg::ui::EVAlignType::eBottom;
				}
				else if (alignmentStr == "center")
				{
					baseComp.m_VAlign = pg::ui::EVAlignType::eCenter;
				}
			}

			if (jsonObject.contains("uuid"))
			{
				baseComp.m_UUID = pg::UUID(jsonObject["uuid"].get<std::string>());
			}

			accessor.emplace_deferred<pg::ui::BaseComponent>(ent, std::move(baseComp));

			if (jsonObject.contains("children"))
			{
				PG_CORE_EXCEPT(jsonObject["children"].is_array(), "unable to parse json, children is not a array");
				for (json child : jsonObject["children"])
				{
					ParseJsonUIElement(accessor, child, ent);
				}
			}

			if (uiJsonObject.contains("image") && uiJsonObject["image"].is_object())
			{
				PG_CORE_EXCEPT(!uiJsonObject.contains("text"), "we should have either a image or a text but not both");
				ParseImageComponentFromJson(accessor, uiJsonObject["image"], ent);
			}
			if (uiJsonObject.contains("text") && uiJsonObject["text"].is_object())
			{
				PG_CORE_EXCEPT(!uiJsonObject.contains("image"), "we should have either a image or a text but not both");
				ParseTextComponentFromJson(accessor, uiJsonObject["text"], ent);
			}
		}
		else if (jsonObject.contains("uiFile") && jsonObject["uiFile"].is_string())
		{
			std::string path = jsonObject["uiFile"].get<std::string>();
			const std::string jsonLoadedString = ReadFileToString(path);
			json jsonLoadedObject = json::parse(jsonLoadedString);
			ParseJsonUIElement(accessor, jsonLoadedObject, parent);
		}
	}

	void LoadLayoutFromUUID(const pg::ResourceMapSingletonComponent& resourcesComponent, const pg::UUID& layoutID)
	{
		PG_CORE_EXCEPT(resourcesComponent.m_UILayoutMap.find(layoutID) != resourcesComponent.m_UILayoutMap.end(), "Could not find layout");
		const std::string jsonString = ReadFileToString(resourcesComponent.m_UILayoutMap.at(layoutID));
		json jsonObject = json::parse(jsonString);
		auto accessor = pg::World::GetRegistry();
		ParseJsonUIElement(accessor, jsonObject, pg::ecs::null);
	}

	void LoadLayoutFromFile(const std::string& path)
	{
		const std::string jsonString = ReadFileToString(path);
		json jsonObject = json::parse(jsonString);
		auto accessor = pg::World::GetRegistry();
		ParseJsonUIElement(accessor, jsonObject, pg::ecs::null);
	}

	

	
}

pg::SystemAccessDecl pg::ui::UIControlSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::ui::UIUpdateTransformOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateParentOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateEnableOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateUUIDOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateImageUUIDOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateTextOneFrameComponent)),
		std::type_index(typeid(pg::ui::LoadLayoutEvent)),
		std::type_index(typeid(pg::ui::UIDestroyOneFrameComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::ImageComponent)),
		std::type_index(typeid(pg::ui::TextComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::ImageComponent)),
		std::type_index(typeid(pg::ui::TextComponent)),
		std::type_index(typeid(pg::ui::UIUpdateTransformOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateParentOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateEnableOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateUUIDOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateImageUUIDOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateTextOneFrameComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::ImageComponent)),
		std::type_index(typeid(pg::ui::TextComponent)),
	};
	return decl;
}

void pg::ui::UIControlSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto resourcesView = accessor.view<const pg::ResourceMapSingletonComponent>();
	if (resourcesView.empty())
	{
		return;
	}
	const pg::ResourceMapSingletonComponent& resourcesComponent = resourcesView.get<const pg::ResourceMapSingletonComponent>(resourcesView.front());

	auto viewTransform = accessor.view<pg::ui::BaseComponent, const pg::ui::UIUpdateTransformOneFrameComponent>();
	for (auto ent : viewTransform)
	{
		pg::ui::BaseComponent& baseComponent = viewTransform.get<pg::ui::BaseComponent>(ent);
		const pg::ui::UIUpdateTransformOneFrameComponent& updateComponent = viewTransform.get<const pg::ui::UIUpdateTransformOneFrameComponent>(ent);
		baseComponent.m_HAlign = updateComponent.m_HAlign;
		baseComponent.m_VAlign = updateComponent.m_VAlign;
		baseComponent.m_Size = updateComponent.m_Size;
		baseComponent.m_Spacing = updateComponent.m_Spacing;
	}

	auto viewParent = accessor.view<pg::ui::BaseComponent, const pg::ui::UIUpdateParentOneFrameComponent>();
	for (auto ent : viewParent)
	{
		pg::ui::BaseComponent& baseComponent = viewParent.get<pg::ui::BaseComponent>(ent);
		const pg::ui::UIUpdateParentOneFrameComponent& updateComponent = viewParent.get<const pg::ui::UIUpdateParentOneFrameComponent>(ent);
		baseComponent.m_Parent = updateComponent.m_Parent;
	}

	auto viewEnable = accessor.view<pg::ui::BaseComponent, const pg::ui::UIUpdateEnableOneFrameComponent>();
	for (auto ent : viewEnable)
	{
		pg::ui::BaseComponent& baseComponent = viewEnable.get<pg::ui::BaseComponent>(ent);
		const pg::ui::UIUpdateEnableOneFrameComponent& updateComponent = viewEnable.get<const pg::ui::UIUpdateEnableOneFrameComponent>(ent);
		baseComponent.m_Enabled = updateComponent.m_Enabled;
	}

	auto viewUUID = accessor.view<pg::ui::BaseComponent, const pg::ui::UIUpdateUUIDOneFrameComponent>();
	for (auto ent : viewUUID)
	{
		pg::ui::BaseComponent& baseComponent = viewUUID.get<pg::ui::BaseComponent>(ent);
		const pg::ui::UIUpdateUUIDOneFrameComponent& updateComponent = viewUUID.get<const pg::ui::UIUpdateUUIDOneFrameComponent>(ent);
		baseComponent.m_UUID = updateComponent.m_UUID;
	}

	auto viewImage = accessor.view<pg::ui::ImageComponent, const pg::ui::UIUpdateImageUUIDOneFrameComponent>();
	for (auto ent : viewImage)
	{
		pg::ui::ImageComponent& imageComponent = viewImage.get<pg::ui::ImageComponent>(ent);
		const pg::ui::UIUpdateImageUUIDOneFrameComponent& updateComponent = viewImage.get<const pg::ui::UIUpdateImageUUIDOneFrameComponent>(ent);
		imageComponent.m_TextureHandle = updateComponent.m_UUID;
	}

	auto viewText = accessor.view<pg::ui::TextComponent, const pg::ui::UIUpdateTextOneFrameComponent>();
	for (auto ent : viewText)
	{
		pg::ui::TextComponent& textComponent = viewText.get<pg::ui::TextComponent>(ent);
		const pg::ui::UIUpdateTextOneFrameComponent& updateComponent = viewText.get<const pg::ui::UIUpdateTextOneFrameComponent>(ent);
		textComponent.m_FontID = updateComponent.m_FontID;
		textComponent.m_Color = updateComponent.m_Color;
		textComponent.m_Kerning = updateComponent.m_Kerning;
		textComponent.m_Spacing = updateComponent.m_Spacing;
		textComponent.m_Text = updateComponent.m_Text;
	}

	auto viewLoad = accessor.view<const pg::ui::LoadLayoutEvent>();
	for (auto entJson : viewLoad)
	{
		LoadLayoutFromUUID(resourcesComponent, viewLoad.get<const pg::ui::LoadLayoutEvent>(entJson).m_UUID);
	}

	auto viewDestroy = accessor.view<const pg::ui::UIDestroyOneFrameComponent>();
	for (auto ent : viewDestroy)
	{
		DestroyUI(accessor, ent);
	}
}


