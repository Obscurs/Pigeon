#pragma once
#include <entt/entt.hpp>

namespace pg
{
	class CheckedRegistryAccessor;
}
namespace pg::ui
{
	struct BaseComponent;
	struct RendererConfigSingletonComponent;
	struct TextComponent;

	glm::vec4 GetGlobalBoundsForElementParent(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent, int& level);

	glm::vec4 GetGlobalBoundsForElement(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent, const glm::vec2& uiBoundsSize, int& level);

	bool IsUIElementEnabled(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent);
	std::vector<entt::entity> GetUIChildrenForElement(pg::CheckedRegistryAccessor& accessor, entt::entity ent);
}
