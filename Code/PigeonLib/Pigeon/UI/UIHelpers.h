#pragma once
#include <entt/entt.hpp>

namespace pig
{
	class CheckedRegistryAccessor;
}
namespace pig::ui
{
	struct BaseComponent;
	struct RendererConfigSingletonComponent;
	struct TextComponent;

	glm::vec4 GetGlobalBoundsForElementParent(pig::CheckedRegistryAccessor& accessor, const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfigSingletonComponent& renderComponent, int& level);

	glm::vec4 GetGlobalBoundsForElement(pig::CheckedRegistryAccessor& accessor, const pig::ui::BaseComponent& baseComponent, const pig::ui::RendererConfigSingletonComponent& renderComponent, const glm::vec2& uiBoundsSize, int& level);

	bool IsUIElementEnabled(pig::CheckedRegistryAccessor& accessor, const pig::ui::BaseComponent& baseComponent);
	std::vector<entt::entity> GetUIChildrenForElement(pig::CheckedRegistryAccessor& accessor, entt::entity ent);
}
