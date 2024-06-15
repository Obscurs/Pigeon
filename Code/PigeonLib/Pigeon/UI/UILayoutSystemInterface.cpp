#include "UILayoutSystemInterface.h"

#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

#include "Pigeon/ECS/World.h"

pig::ui::UILayoutSystemInterface::UILayoutSystemInterface()
	: pig::System(pig::SystemType::eTest)
{
}

void pig::ui::UILayoutSystemInterface::Update(float dt)
{
}

void pig::ui::UILayoutSystemInterface::LoadLayout(const std::string& path)
{
	entt::entity ent = pig::World::GetRegistry().create();
	pig::ui::LoadLayoutOneFrameComponent& comp = pig::World::GetRegistry().emplace<pig::ui::LoadLayoutOneFrameComponent>(ent);
	comp.m_LayoutFilePath = path;
}


void pig::ui::UILayoutSystemInterface::DestroyLayout(const pig::UUID& layoutId)
{
	pig::World::GetRegistry().emplace<pig::ui::UIDestroyOneFrameComponent>(pig::World::GetRegistry().create());
}

void pig::ui::UILayoutSystemInterface::UpdateLayoutTransform(const pig::UUID& layoutId, const glm::vec2& size, const glm::vec2& spacing, pig::ui::EHAlignType hvalue, pig::ui::EVAlignType vvalue)
{
	int count = 0;
	auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
	for (auto ent : viewUI)
	{
		const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
		if (baseComponent.m_UUID == layoutId)
		{
			count++;
			pig::ui::UIUpdateTransformOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateTransformOneFrameComponent>(ent);
			updateComponent.m_HAlign = hvalue;
			updateComponent.m_VAlign = vvalue;
			updateComponent.m_Spacing = spacing;
			updateComponent.m_Size = size;
		}
	}
	PG_CORE_ASSERT(count > 0, "UI element not found");
	PG_CORE_ASSERT(count <= 1, "There are more than one UI elements with the same uuid");
}

void pig::ui::UILayoutSystemInterface::UpdateLayoutUUID(const pig::UUID& layoutId, const pig::UUID& value)
{
	int count = 0;
	auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
	for (auto ent : viewUI)
	{
		const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
		if (baseComponent.m_UUID == layoutId)
		{
			count++;
			pig::ui::UIUpdateUUIDOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateUUIDOneFrameComponent>(ent);
			updateComponent.m_UUID = value;
		}
	}
	PG_CORE_ASSERT(count > 0, "UI element not found");
	PG_CORE_ASSERT(count <= 1, "There are more than one UI elements with the same uuid");
}

void pig::ui::UILayoutSystemInterface::UpdateLayoutParent(const pig::UUID& layoutId, entt::entity value)
{
	int count = 0;
	auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
	for (auto ent : viewUI)
	{
		const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
		if (baseComponent.m_UUID == layoutId)
		{
			count++;
			pig::ui::UIUpdateParentOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateParentOneFrameComponent>(ent);
			updateComponent.m_Parent = value;
		}
	}
	PG_CORE_ASSERT(count > 0, "UI element not found");
	PG_CORE_ASSERT(count <= 1, "There are more than one UI elements with the same uuid");
}

void pig::ui::UILayoutSystemInterface::UpdateLayoutTexture(const pig::UUID& layoutId, const pig::UUID& value, bool destroyPrevious)
{
	int count = 0;
	auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::ImageComponent>();
	for (auto ent : viewUI)
	{
		const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
		const pig::ui::ImageComponent& imageComponent = viewUI.get<pig::ui::ImageComponent>(ent);
		if (baseComponent.m_UUID == layoutId)
		{
			count++;
			pig::ui::UIUpdateImageUUIDOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateImageUUIDOneFrameComponent>(ent);
			if (destroyPrevious)
				updateComponent.m_PreviousImageToDestroy = imageComponent.m_TextureHandle;
			updateComponent.m_UUID = value;
		}
	}
	PG_CORE_ASSERT(count > 0, "UI element not found");
	PG_CORE_ASSERT(count <= 1, "There are more than one UI elements with the same uuid");
}

void pig::ui::UILayoutSystemInterface::UpdateLayoutText(const pig::UUID& layoutId, const std::string& value, const glm::vec4& color)
{
	int count = 0;
	auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::TextComponent>();
	for (auto ent : viewUI)
	{
		const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
		const pig::ui::TextComponent& textComponent = viewUI.get<pig::ui::TextComponent>(ent);
		if (baseComponent.m_UUID == layoutId)
		{
			count++;
			pig::ui::UIUpdateTextOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateTextOneFrameComponent>(ent);
			updateComponent.m_Text = value;
			updateComponent.m_Color = color;
			updateComponent.m_Kerning = textComponent.m_Kerning;
			updateComponent.m_Spacing = textComponent.m_Spacing;
		}
	}
	PG_CORE_ASSERT(count > 0, "UI element not found");
	PG_CORE_ASSERT(count <= 1, "There are more than one UI elements with the same uuid");
}

void pig::ui::UILayoutSystemInterface::UpdateEnable(const pig::UUID& layoutId, bool value)
{
	int count = 0;
	auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
	for (auto ent : viewUI)
	{
		const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
		if (baseComponent.m_UUID == layoutId)
		{
			count++;
			pig::ui::UIUpdateEnableOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateEnableOneFrameComponent>(ent);
			updateComponent.m_Enabled = value;
		}
	}
	PG_CORE_ASSERT(count > 0, "UI element not found");
	PG_CORE_ASSERT(count <= 1, "There are more than one UI elements with the same uuid");
}