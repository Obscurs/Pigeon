#pragma once
#include <entt/entt.hpp>

namespace pig::ui
{
	struct BaseComponent;
	struct RendererConfig;
	struct TextComponent;

	glm::vec4 GetGlobalBoundsForElementParent(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, int& DEPRECATED_level);

	glm::vec4 GetGlobalBoundsForElement(const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, const glm::vec2& uiBoundsSize, int& DEPRECATED_level);

	bool IsUIElementEnabled(const pig::ui::BaseComponent& baseComponent);
	std::vector<entt::entity> GetUIChildrenForElement(entt::entity ent);
}
