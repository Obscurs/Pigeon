#include "UIHelpers.h"

#include "Pigeon/UI/UIComponents.h"

glm::vec4 pg::ui::GetGlobalBoundsForElementParent(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent, int& level)
{
	glm::vec4 globalBounds(0.f, 0.f, renderComponent.m_Width, renderComponent.m_Height);

	if (baseComponent.m_Parent != pg::ecs::null && accessor.any_of<pg::ui::BaseComponent>(baseComponent.m_Parent))
	{
		const pg::ui::BaseComponent& parentComponent = accessor.get<const pg::ui::BaseComponent>(baseComponent.m_Parent);
		level += 1;
		globalBounds = GetGlobalBoundsForElementParent(accessor, parentComponent, renderComponent, level);

		if (parentComponent.m_HAlign == pg::ui::EHAlignType::eRight)
		{
			globalBounds.x += globalBounds.z - (parentComponent.m_Spacing.x + parentComponent.m_Size.x);
		}
		else if (parentComponent.m_HAlign == pg::ui::EHAlignType::eCenter)
		{
			globalBounds.x += globalBounds.z / 2.f - parentComponent.m_Size.x / 2.f + parentComponent.m_Spacing.x;
		}
		else
		{
			globalBounds.x += parentComponent.m_Spacing.x;
		}

		if (parentComponent.m_VAlign == pg::ui::EVAlignType::eBottom)
		{
			globalBounds.y += globalBounds.w - (parentComponent.m_Spacing.y + parentComponent.m_Size.y);
		}
		else if (parentComponent.m_VAlign == pg::ui::EVAlignType::eCenter)
		{
			globalBounds.y += globalBounds.w / 2.f - parentComponent.m_Size.y / 2.f + parentComponent.m_Spacing.y;
		}
		else
		{
			globalBounds.y += parentComponent.m_Spacing.y;
		}

		globalBounds.z = parentComponent.m_Size.x;
		globalBounds.w = parentComponent.m_Size.y;
	}

	return globalBounds;
}

glm::vec4 pg::ui::GetGlobalBoundsForElement(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent, const glm::vec2& uiBoundsSize, int& level)
{
	const glm::vec4 bounds = pg::ui::GetGlobalBoundsForElementParent(accessor, baseComponent, renderComponent, level);
	glm::vec2 posFinal = glm::vec2(bounds.x, bounds.y);

	if (baseComponent.m_HAlign == EHAlignType::eRight)
	{
		posFinal.x += bounds.z - (uiBoundsSize.x + baseComponent.m_Spacing.x);
	}
	else if (baseComponent.m_HAlign == EHAlignType::eCenter)
	{
		posFinal.x += (bounds.z / 2.f - uiBoundsSize.x / 2.f) + baseComponent.m_Spacing.x;
	}
	else
	{
		posFinal.x += baseComponent.m_Spacing.x;
	}

	if (baseComponent.m_VAlign == EVAlignType::eBottom)
	{
		posFinal.y += bounds.w - (uiBoundsSize.y + baseComponent.m_Spacing.y);
	}
	else if (baseComponent.m_VAlign == EVAlignType::eCenter)
	{
		posFinal.y += (bounds.w / 2.f - uiBoundsSize.y / 2.f) + baseComponent.m_Spacing.y;
	}
	else
	{
		posFinal.y += baseComponent.m_Spacing.y;
	}
	return glm::vec4(posFinal, uiBoundsSize);
}

bool pg::ui::IsUIElementEnabled(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent)
{
	bool enabled = baseComponent.m_Enabled;
	pg::ecs::Entity parentEntity = baseComponent.m_Parent;
	while (enabled && parentEntity != pg::ecs::null)
	{
		PG_CORE_EXCEPT(accessor.any_of<pg::ui::BaseComponent>(parentEntity), "parent entity does not have base component");
		const pg::ui::BaseComponent& parentComponent = accessor.get<const pg::ui::BaseComponent>(parentEntity);
		enabled = parentComponent.m_Enabled;
		parentEntity = parentComponent.m_Parent;
	}
	return enabled;
}

std::vector<pg::ecs::Entity> pg::ui::GetUIChildrenForElement(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent)
{
	std::vector<pg::ecs::Entity> children{};
	auto view = accessor.view<const pg::ui::BaseComponent>();
	for (auto child : view)
	{
		const pg::ui::BaseComponent& baseComponent = view.get<const pg::ui::BaseComponent>(child);
		if (baseComponent.m_Parent == ent)
			children.push_back(child);
	}
	return children;
}
