#pragma once
#include "Pigeon/ECS/Entity.h"

namespace pg
{
	class CheckedRegistryAccessor;
}
namespace pg::ui
{
	struct BaseComponent;
	struct RendererConfigSingletonComponent;
	struct TextComponent;

	// Resolves an element's screen rect (xy = top-left, zw = size) in logical-canvas units from the
	// anchor-rect model, recursing up the parent chain; the root element's parent is the full logical
	// canvas (0,0,m_Width,m_Height). The single geometry UI rendering and hit-testing both read.
	glm::vec4 GetElementRect(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent);

	// Nesting depth (0 = direct child of the canvas), used to order the UI draw pass (children over parents).
	int GetElementDepth(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent);

	// The UI camera's vertical orthographic bounds {bottom, top} for a logical-canvas height. The UI canvas
	// is y-down (y=0 at the top); the camera uses an inverted ortho so canvas y=0 maps to the TOP of the
	// screen and y=height to the bottom: returns {bottom, top} = {height, 0}. Backend independent — world
	// space is Y-up via the camera projections, not a renderer vertex negation.
	glm::vec2 GetUICameraOrthoBottomTop(float height);

	// The element's effective clip rect in logical-canvas units: the intersection of all ancestor clip
	// regions (elements carrying a UIClipComponent), or the full logical canvas when there is no ancestor
	// clip. UI rendering masks the element's draw to this rect.
	glm::vec4 GetElementClipRect(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent);

	bool IsUIElementEnabled(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent);
	std::vector<pg::ecs::Entity> GetUIChildrenForElement(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent);
}
