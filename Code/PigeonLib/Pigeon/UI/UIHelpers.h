#pragma once
#include <entt/entt.hpp>

namespace pig
{
	class CheckedRegistryAccessor;
}
namespace pig::ui
{
	struct BaseComponent;
	struct RendererConfig;
	struct TextComponent;

	glm::vec4 GetGlobalBoundsForElementParent(pig::CheckedRegistryAccessor& accessor, const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, int& DEPRECATED_level);

	glm::vec4 GetGlobalBoundsForElement(pig::CheckedRegistryAccessor& accessor, const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfig& renderComponent, const glm::vec2& uiBoundsSize, int& DEPRECATED_level);

	bool IsUIElementEnabled(pig::CheckedRegistryAccessor& accessor, const pig::ui::BaseComponent& baseComponent);
	std::vector<entt::entity> GetUIChildrenForElement(pig::CheckedRegistryAccessor& accessor, entt::entity ent);
}
