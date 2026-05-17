#pragma once
#include <entt/entt.hpp>

namespace pig::ui
{
	struct BaseComponent;
	struct RendererConfig;
	struct TextComponent;

	glm::vec4 GetGlobalBoundsForElementParent(entt::registry& reg, const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, int& DEPRECATED_level);

	glm::vec4 GetGlobalBoundsForElement(entt::registry& reg, const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, const glm::vec2& uiBoundsSize, int& DEPRECATED_level);

	bool IsUIElementEnabled(entt::registry& reg, const pig::ui::BaseComponent& baseComponent);
	std::vector<entt::entity> GetUIChildrenForElement(entt::registry& reg, entt::entity ent);
}
