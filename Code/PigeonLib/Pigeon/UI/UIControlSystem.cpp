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

	glm::vec2 ParseVec2(const json& arr, const glm::vec2& fallback)
	{
		if (arr.is_array() && arr.size() == 2 && arr[0].is_number() && arr[1].is_number())
		{
			return glm::vec2(arr[0].get<float>(), arr[1].get<float>());
		}
		return fallback;
	}

	void DestroyUI(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent)
	{
		std::vector<pg::ecs::Entity> children = pg::ui::GetUIChildrenForElement(accessor, ent);
		for (pg::ecs::Entity& child : children)
		{
			DestroyUI(accessor, child);
		}
		accessor.DestroyDeferred(ent);
	}

	void ParseImageComponentFromJson(pg::CheckedRegistryAccessor& accessor, const json& jsonObject, pg::ecs::Entity ent)
	{
		pg::ui::ImageComponent component;
		if (jsonObject.contains("id"))
		{
			PG_CORE_EXCEPT(jsonObject["id"].is_string(), "unable to parse json, image id is not a string");
			component.m_TextureHandle = pg::UUID(jsonObject["id"].get<std::string>());
		}
		if (jsonObject.contains("border"))
		{
			const json& border = jsonObject["border"];
			if (border.is_array() && border.size() == 4 && border[0].is_number() && border[1].is_number() && border[2].is_number() && border[3].is_number())
			{
				component.m_NineSliceBorder = glm::vec4(border[0].get<float>(), border[1].get<float>(), border[2].get<float>(), border[3].get<float>());
			}
		}

		accessor.EmplaceDeferred<pg::ui::ImageComponent>(ent, std::move(component));
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
		if (jsonObject.contains("align_h") && jsonObject["align_h"].is_string())
		{
			const std::string alignStr = jsonObject["align_h"].get<std::string>();
			if (alignStr == "left")        component.m_HAlign = pg::ui::EHAlignType::eLeft;
			else if (alignStr == "right")  component.m_HAlign = pg::ui::EHAlignType::eRight;
			else if (alignStr == "center") component.m_HAlign = pg::ui::EHAlignType::eCenter;
		}
		if (jsonObject.contains("align_v") && jsonObject["align_v"].is_string())
		{
			const std::string alignStr = jsonObject["align_v"].get<std::string>();
			if (alignStr == "top")         component.m_VAlign = pg::ui::EVAlignType::eTop;
			else if (alignStr == "bottom") component.m_VAlign = pg::ui::EVAlignType::eBottom;
			else if (alignStr == "center") component.m_VAlign = pg::ui::EVAlignType::eCenter;
		}
		if (jsonObject.contains("fontSize"))
		{
			PG_CORE_EXCEPT(jsonObject["fontSize"].is_number(), "unable to parse json, fontSize is not a number");
			component.m_FixedFontSize = jsonObject["fontSize"].get<float>();
		}
		if (jsonObject.contains("wrap"))
		{
			PG_CORE_EXCEPT(jsonObject["wrap"].is_boolean(), "unable to parse json, wrap is not a boolean");
			component.m_WordWrap = jsonObject["wrap"].get<bool>();
		}
		if (jsonObject.contains("visibleChars"))
		{
			PG_CORE_EXCEPT(jsonObject["visibleChars"].is_number_integer(), "unable to parse json, visibleChars is not an integer");
			component.m_VisibleChars = jsonObject["visibleChars"].get<int>();
		}
		accessor.EmplaceDeferred<pg::ui::TextComponent>(ent, std::move(component));
	}

	void ParseLayoutContainerFromJson(pg::CheckedRegistryAccessor& accessor, const json& jsonObject, pg::ecs::Entity ent)
	{
		pg::ui::LayoutContainerComponent container;
		if (jsonObject.contains("type") && jsonObject["type"].is_string())
		{
			const std::string type = jsonObject["type"].get<std::string>();
			if (type == "vertical")        container.m_Type = pg::ui::ELayoutType::eVertical;
			else if (type == "horizontal") container.m_Type = pg::ui::ELayoutType::eHorizontal;
			else if (type == "grid")       container.m_Type = pg::ui::ELayoutType::eGrid;
		}
		if (jsonObject.contains("padding"))
		{
			const json& padding = jsonObject["padding"];
			if (padding.is_array() && padding.size() == 4 && padding[0].is_number() && padding[1].is_number() && padding[2].is_number() && padding[3].is_number())
			{
				container.m_Padding = glm::vec4(padding[0].get<float>(), padding[1].get<float>(), padding[2].get<float>(), padding[3].get<float>());
			}
		}
		if (jsonObject.contains("spacing"))
		{
			container.m_Spacing = ParseVec2(jsonObject["spacing"], container.m_Spacing);
		}
		if (jsonObject.contains("columns") && jsonObject["columns"].is_number_integer())
		{
			container.m_Columns = jsonObject["columns"].get<int>();
		}
		if (jsonObject.contains("cellSize"))
		{
			container.m_CellSize = ParseVec2(jsonObject["cellSize"], container.m_CellSize);
		}
		accessor.EmplaceDeferred<pg::ui::LayoutContainerComponent>(ent, std::move(container));
	}

	void ParseJsonUIElement(pg::CheckedRegistryAccessor& accessor, const json& jsonObject, pg::ecs::Entity parent, int siblingIndex)
	{
		if (jsonObject.contains("ui") && jsonObject["ui"].is_object())
		{
			const json& uiJsonObject = jsonObject["ui"];
			pg::ecs::Entity ent = accessor.Create();
			pg::ui::BaseComponent baseComp;
			baseComp.m_Parent = parent;
			baseComp.m_SiblingIndex = siblingIndex;

			// Anchor-rect layout: all fields optional, [x, y] arrays; absent keys keep the
			// component defaults (a top-left point anchor).
			if (jsonObject.contains("anchorMin"))
			{
				baseComp.m_AnchorMin = ParseVec2(jsonObject["anchorMin"], baseComp.m_AnchorMin);
			}
			if (jsonObject.contains("anchorMax"))
			{
				baseComp.m_AnchorMax = ParseVec2(jsonObject["anchorMax"], baseComp.m_AnchorMax);
			}
			if (jsonObject.contains("pivot"))
			{
				baseComp.m_Pivot = ParseVec2(jsonObject["pivot"], baseComp.m_Pivot);
			}
			if (jsonObject.contains("anchoredPosition"))
			{
				baseComp.m_AnchoredPosition = ParseVec2(jsonObject["anchoredPosition"], baseComp.m_AnchoredPosition);
			}
			if (jsonObject.contains("size"))
			{
				baseComp.m_Size = ParseVec2(jsonObject["size"], baseComp.m_Size);
			}

			if (jsonObject.contains("uuid"))
			{
				baseComp.m_UUID = pg::UUID(jsonObject["uuid"].get<std::string>());
			}

			accessor.EmplaceDeferred<pg::ui::BaseComponent>(ent, std::move(baseComp));

			if (jsonObject.contains("layout") && jsonObject["layout"].is_object())
			{
				ParseLayoutContainerFromJson(accessor, jsonObject["layout"], ent);
			}

			if (jsonObject.contains("clip") && jsonObject["clip"].is_object())
			{
				pg::ui::UIClipComponent clip;
				if (jsonObject["clip"].contains("scrollOffset"))
				{
					clip.m_ScrollOffset = ParseVec2(jsonObject["clip"]["scrollOffset"], clip.m_ScrollOffset);
				}
				accessor.EmplaceDeferred<pg::ui::UIClipComponent>(ent, std::move(clip));
			}

			if (jsonObject.contains("children"))
			{
				PG_CORE_EXCEPT(jsonObject["children"].is_array(), "unable to parse json, children is not a array");
				int childIndex = 0;
				for (json child : jsonObject["children"])
				{
					ParseJsonUIElement(accessor, child, ent, childIndex);
					++childIndex;
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
			ParseJsonUIElement(accessor, jsonLoadedObject, parent, siblingIndex);
		}
	}

	void LoadLayoutFromUUID(const pg::ResourceMapSingletonComponent& resourcesComponent, const pg::UUID& layoutID)
	{
		PG_CORE_EXCEPT(resourcesComponent.m_UILayoutMap.find(layoutID) != resourcesComponent.m_UILayoutMap.end(), "Could not find layout");
		const std::string jsonString = ReadFileToString(resourcesComponent.m_UILayoutMap.at(layoutID));
		json jsonObject = json::parse(jsonString);
		auto accessor = pg::World::GetRegistry();
		ParseJsonUIElement(accessor, jsonObject, pg::ecs::null, 0);
	}

	void LoadLayoutFromFile(const std::string& path)
	{
		const std::string jsonString = ReadFileToString(path);
		json jsonObject = json::parse(jsonString);
		auto accessor = pg::World::GetRegistry();
		ParseJsonUIElement(accessor, jsonObject, pg::ecs::null, 0);
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
		std::type_index(typeid(pg::ui::UIUpdateTextRevealOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateClipOffsetOneFrameComponent)),
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
		std::type_index(typeid(pg::ui::UIClipComponent)),
		std::type_index(typeid(pg::ui::UIUpdateTransformOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateParentOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateEnableOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateUUIDOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateImageUUIDOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateTextOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIUpdateClipOffsetOneFrameComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::ImageComponent)),
		std::type_index(typeid(pg::ui::TextComponent)),
		std::type_index(typeid(pg::ui::LayoutContainerComponent)),
		std::type_index(typeid(pg::ui::UIClipComponent)),
	};
	return decl;
}

void pg::ui::UIControlSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto resourcesView = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (resourcesView.empty())
	{
		return;
	}
	const pg::ResourceMapSingletonComponent& resourcesComponent = resourcesView.get<const pg::ResourceMapSingletonComponent>(resourcesView.front());

	auto viewTransform = accessor.View<pg::ui::BaseComponent, const pg::ui::UIUpdateTransformOneFrameComponent>();
	for (auto ent : viewTransform)
	{
		pg::ui::BaseComponent& baseComponent = viewTransform.get<pg::ui::BaseComponent>(ent);
		const pg::ui::UIUpdateTransformOneFrameComponent& updateComponent = viewTransform.get<const pg::ui::UIUpdateTransformOneFrameComponent>(ent);
		baseComponent.m_AnchorMin = updateComponent.m_AnchorMin;
		baseComponent.m_AnchorMax = updateComponent.m_AnchorMax;
		baseComponent.m_Pivot = updateComponent.m_Pivot;
		baseComponent.m_AnchoredPosition = updateComponent.m_AnchoredPosition;
		baseComponent.m_Size = updateComponent.m_Size;
	}

	auto viewParent = accessor.View<pg::ui::BaseComponent, const pg::ui::UIUpdateParentOneFrameComponent>();
	for (auto ent : viewParent)
	{
		pg::ui::BaseComponent& baseComponent = viewParent.get<pg::ui::BaseComponent>(ent);
		const pg::ui::UIUpdateParentOneFrameComponent& updateComponent = viewParent.get<const pg::ui::UIUpdateParentOneFrameComponent>(ent);
		baseComponent.m_Parent = updateComponent.m_Parent;
	}

	auto viewEnable = accessor.View<pg::ui::BaseComponent, const pg::ui::UIUpdateEnableOneFrameComponent>();
	for (auto ent : viewEnable)
	{
		pg::ui::BaseComponent& baseComponent = viewEnable.get<pg::ui::BaseComponent>(ent);
		const pg::ui::UIUpdateEnableOneFrameComponent& updateComponent = viewEnable.get<const pg::ui::UIUpdateEnableOneFrameComponent>(ent);
		baseComponent.m_Enabled = updateComponent.m_Enabled;
	}

	auto viewUUID = accessor.View<pg::ui::BaseComponent, const pg::ui::UIUpdateUUIDOneFrameComponent>();
	for (auto ent : viewUUID)
	{
		pg::ui::BaseComponent& baseComponent = viewUUID.get<pg::ui::BaseComponent>(ent);
		const pg::ui::UIUpdateUUIDOneFrameComponent& updateComponent = viewUUID.get<const pg::ui::UIUpdateUUIDOneFrameComponent>(ent);
		baseComponent.m_UUID = updateComponent.m_UUID;
	}

	auto viewClipOffset = accessor.View<pg::ui::UIClipComponent, const pg::ui::UIUpdateClipOffsetOneFrameComponent>();
	for (auto ent : viewClipOffset)
	{
		pg::ui::UIClipComponent& clipComponent = viewClipOffset.get<pg::ui::UIClipComponent>(ent);
		const pg::ui::UIUpdateClipOffsetOneFrameComponent& updateComponent = viewClipOffset.get<const pg::ui::UIUpdateClipOffsetOneFrameComponent>(ent);
		clipComponent.m_ScrollOffset = updateComponent.m_ScrollOffset;
	}

	auto viewImage = accessor.View<pg::ui::ImageComponent, const pg::ui::UIUpdateImageUUIDOneFrameComponent>();
	for (auto ent : viewImage)
	{
		pg::ui::ImageComponent& imageComponent = viewImage.get<pg::ui::ImageComponent>(ent);
		const pg::ui::UIUpdateImageUUIDOneFrameComponent& updateComponent = viewImage.get<const pg::ui::UIUpdateImageUUIDOneFrameComponent>(ent);
		imageComponent.m_TextureHandle = updateComponent.m_UUID;
	}

	auto viewText = accessor.View<pg::ui::TextComponent, const pg::ui::UIUpdateTextOneFrameComponent>();
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

	auto viewTextReveal = accessor.View<pg::ui::TextComponent, const pg::ui::UIUpdateTextRevealOneFrameComponent>();
	for (auto ent : viewTextReveal)
	{
		pg::ui::TextComponent& textComponent = viewTextReveal.get<pg::ui::TextComponent>(ent);
		const pg::ui::UIUpdateTextRevealOneFrameComponent& updateComponent = viewTextReveal.get<const pg::ui::UIUpdateTextRevealOneFrameComponent>(ent);
		// A non-empty text swaps in a new dialogue line; an empty one advances the reveal only.
		if (!updateComponent.m_Text.empty())
		{
			textComponent.m_Text = updateComponent.m_Text;
		}
		textComponent.m_VisibleChars = updateComponent.m_VisibleChars;
	}

	auto viewLoad = accessor.View<const pg::ui::LoadLayoutEvent>();
	for (auto entJson : viewLoad)
	{
		LoadLayoutFromUUID(resourcesComponent, viewLoad.get<const pg::ui::LoadLayoutEvent>(entJson).m_UUID);
	}

	auto viewDestroy = accessor.View<const pg::ui::UIDestroyOneFrameComponent>();
	for (auto ent : viewDestroy)
	{
		DestroyUI(accessor, ent);
	}
}


