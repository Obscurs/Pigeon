#include "LayoutControlSystem.h"

#include "Pigeon/UI/UIComponents.h"

#include "Pigeon/ECS/World.h"

pig::ui::LayoutControlSystem::LayoutControlSystem():
	pig::System(pig::SystemType::eTest)
{
}

void pig::ui::LayoutControlSystem::Update(float dt)
{
	auto view = pig::World::GetRegistry().view<const pig::ui::LoadableLayoutOneFrameComponent>();

	for (auto ent : view)
	{
		entt::entity layoutEntity = pig::World::GetRegistry().create();
		pig::ui::BaseComponent& baseComponent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(layoutEntity);
		pig::ui::TextComponent& textComponent = pig::World::GetRegistry().emplace<pig::ui::TextComponent>(layoutEntity);

		baseComponent.m_Size.x = 400.f;
		baseComponent.m_Size.y = 50;

		baseComponent.m_Spacing.x = 5;
		baseComponent.m_Spacing.y = 10;

		baseComponent.m_HAlign = pig::ui::EHAlignType::eLeft;
		baseComponent.m_VAlign = pig::ui::EVAlignType::eNone;

		textComponent.m_Text = std::string("this is a sample text");
	}
}
