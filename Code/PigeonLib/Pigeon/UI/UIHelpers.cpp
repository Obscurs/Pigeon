#include "UIHelpers.h"

#include "Pigeon/UI/UIComponents.h"

#include "Pigeon/ECS/World.h"

glm::vec4 pig::ui::GetGlobalBoundsForElementParent(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, int& DEPRECATED_level)
{
	glm::vec4 globalBounds(0.f, 0.f, renderComponent.m_Width, renderComponent.m_Height);

	if (baseComponent.m_Parent != entt::null && pig::World::GetRegistry().any_of<pig::ui::BaseComponent>(baseComponent.m_Parent))
	{
		const pig::ui::BaseComponent& parentComponent = pig::World::GetRegistry().get<const pig::ui::BaseComponent>(baseComponent.m_Parent);
		DEPRECATED_level += 1;
		globalBounds = GetGlobalBoundsForElementParent(parentComponent, renderComponent, DEPRECATED_level);

		if (parentComponent.m_HAlign == pig::ui::EHAlignType::eRight)
		{
			globalBounds.x += globalBounds.z - (parentComponent.m_Spacing.x + parentComponent.m_Size.x);
		}
		else if (parentComponent.m_HAlign == pig::ui::EHAlignType::eCenter)
		{
			globalBounds.x += globalBounds.z / 2.f - parentComponent.m_Size.x / 2.f + parentComponent.m_Spacing.x;
		}
		else
		{
			globalBounds.x += parentComponent.m_Spacing.x;
		}

		if (parentComponent.m_VAlign == pig::ui::EVAlignType::eBottom)
		{
			globalBounds.y += globalBounds.w - (parentComponent.m_Spacing.y + parentComponent.m_Size.y);
		}
		else if (parentComponent.m_VAlign == pig::ui::EVAlignType::eCenter)
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

glm::vec4 pig::ui::GetGlobalBoundsForElement(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, const glm::vec2& uiBoundsSize, int& DEPRECATED_level)
{
	const glm::vec4 bounds = pig::ui::GetGlobalBoundsForElementParent(baseComponent, renderComponent, DEPRECATED_level);
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
