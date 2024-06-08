#include "UIControlSystem.h"

#include "Pigeon/UI/UIComponents.h"

#include "Pigeon/ECS/World.h"

pig::ui::UIControlSystem::UIControlSystem():
	pig::System(pig::SystemType::eTest)
{
}

// JSON string
		/*std::string jsonString = R"({
			"name": "John Doe",
			"age": 30,
			"is_student": false,
			"address": {
				"street": "123 Main St",
				"city": "Anytown"
			},
			"phone_numbers": ["123-456-7890", "987-654-3210"]
		})";

		// Parse the JSON string
		json jsonObject = json::parse(jsonString);*/
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
}
