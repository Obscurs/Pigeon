#include "Pigeon/UI/UIHelpers.h"

#include "Pigeon/UI/UIComponents.h"

#include <algorithm>

namespace
{
	// Resolves a child rect against a parent rect using the anchor-rect model. Mirrors Unity's
	// RectTransform: offsetMin = anchoredPosition - size*pivot, offsetMax = anchoredPosition + size*(1-pivot),
	// applied relative to the anchor reference points, so rect.size = anchorSpan + size.
	glm::vec4 ResolveRect(const glm::vec4& parentRect, const pg::ui::BaseComponent& baseComponent)
	{
		const glm::vec2 parentMin(parentRect.x, parentRect.y);
		const glm::vec2 parentSize(parentRect.z, parentRect.w);

		const glm::vec2 anchorRefMin = parentMin + baseComponent.m_AnchorMin * parentSize;
		const glm::vec2 anchorRefMax = parentMin + baseComponent.m_AnchorMax * parentSize;

		const glm::vec2 size = (anchorRefMax - anchorRefMin) + baseComponent.m_Size;
		const glm::vec2 rectMin = anchorRefMin + baseComponent.m_AnchoredPosition - baseComponent.m_Size * baseComponent.m_Pivot;

		return glm::vec4(rectMin, size);
	}

	// Computes a child's rect from its parent container layout, overriding the child's anchors.
	// Stacks place children along an axis (accumulating earlier siblings' main-axis sizes + spacing) and
	// stretch them to the content cross-axis; a grid places fixed cells, wrapping after m_Columns columns.
	glm::vec4 ComputeContainerChildRect(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity parentEntity, const glm::vec4& parentRect, const pg::ui::BaseComponent& childComponent, const pg::ui::LayoutContainerComponent& container)
	{
		const float contentX = parentRect.x + container.m_Padding.x;
		const float contentY = parentRect.y + container.m_Padding.y;
		const float contentW = parentRect.z - container.m_Padding.x - container.m_Padding.z;
		const float contentH = parentRect.w - container.m_Padding.y - container.m_Padding.w;

		if (container.m_Type == pg::ui::ELayoutType::eGrid)
		{
			const int columns = container.m_Columns > 0 ? container.m_Columns : 1;
			const int col = childComponent.m_SiblingIndex % columns;
			const int row = childComponent.m_SiblingIndex / columns;
			const float x = contentX + col * (container.m_CellSize.x + container.m_Spacing.x);
			const float y = contentY + row * (container.m_CellSize.y + container.m_Spacing.y);
			return glm::vec4(x, y, container.m_CellSize.x, container.m_CellSize.y);
		}

		const bool vertical = container.m_Type == pg::ui::ELayoutType::eVertical;
		const float gap = vertical ? container.m_Spacing.y : container.m_Spacing.x;

		float offset = 0.f;
		const std::vector<pg::ecs::Entity> siblings = pg::ui::GetUIChildrenForElement(accessor, parentEntity);
		for (pg::ecs::Entity sibling : siblings)
		{
			const pg::ui::BaseComponent& siblingComponent = accessor.Get<const pg::ui::BaseComponent>(sibling);
			if (siblingComponent.m_SiblingIndex < childComponent.m_SiblingIndex)
			{
				offset += (vertical ? siblingComponent.m_Size.y : siblingComponent.m_Size.x) + gap;
			}
		}

		if (vertical)
		{
			return glm::vec4(contentX, contentY + offset, contentW, childComponent.m_Size.y);
		}
		return glm::vec4(contentX + offset, contentY, childComponent.m_Size.x, contentH);
	}
}

glm::vec4 pg::ui::GetElementRect(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent)
{
	glm::vec4 parentRect(0.f, 0.f, renderComponent.m_Width, renderComponent.m_Height);

	if (baseComponent.m_Parent != pg::ecs::null && accessor.AnyOf<pg::ui::BaseComponent>(baseComponent.m_Parent))
	{
		const pg::ui::BaseComponent& parentComponent = accessor.Get<const pg::ui::BaseComponent>(baseComponent.m_Parent);
		parentRect = GetElementRect(accessor, parentComponent, renderComponent);

		// A container parent positions its children itself, overriding the child's own anchor rect.
		glm::vec4 rect;
		if (accessor.AnyOf<pg::ui::LayoutContainerComponent>(baseComponent.m_Parent))
		{
			const pg::ui::LayoutContainerComponent& container = accessor.Get<const pg::ui::LayoutContainerComponent>(baseComponent.m_Parent);
			rect = ComputeContainerChildRect(accessor, baseComponent.m_Parent, parentRect, baseComponent, container);
		}
		else
		{
			rect = ResolveRect(parentRect, baseComponent);
		}

		// A clip parent scrolls its content: shift its direct children by the parent's scroll offset.
		// (Deeper descendants inherit the shift because their parent rect is already shifted.)
		if (accessor.AnyOf<pg::ui::UIClipComponent>(baseComponent.m_Parent))
		{
			const pg::ui::UIClipComponent& clip = accessor.Get<const pg::ui::UIClipComponent>(baseComponent.m_Parent);
			rect.x += clip.m_ScrollOffset.x;
			rect.y += clip.m_ScrollOffset.y;
		}
		return rect;
	}

	return ResolveRect(parentRect, baseComponent);
}

glm::vec4 pg::ui::GetElementClipRect(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent, const pg::ui::RendererConfigSingletonComponent& renderComponent)
{
	glm::vec4 clip(0.f, 0.f, renderComponent.m_Width, renderComponent.m_Height);

	pg::ecs::Entity parentEntity = baseComponent.m_Parent;
	while (parentEntity != pg::ecs::null && accessor.AnyOf<pg::ui::BaseComponent>(parentEntity))
	{
		const pg::ui::BaseComponent& parentComponent = accessor.Get<const pg::ui::BaseComponent>(parentEntity);
		if (accessor.AnyOf<pg::ui::UIClipComponent>(parentEntity))
		{
			const glm::vec4 parentRect = GetElementRect(accessor, parentComponent, renderComponent);

			// Intersect the running clip with this ancestor clip rect (xy = top-left, zw = size).
			const float minX = std::max(clip.x, parentRect.x);
			const float minY = std::max(clip.y, parentRect.y);
			const float maxX = std::min(clip.x + clip.z, parentRect.x + parentRect.z);
			const float maxY = std::min(clip.y + clip.w, parentRect.y + parentRect.w);
			clip = glm::vec4(minX, minY, std::max(0.f, maxX - minX), std::max(0.f, maxY - minY));
		}
		parentEntity = parentComponent.m_Parent;
	}

	return clip;
}

glm::vec2 pg::ui::GetUICameraOrthoBottomTop(float height, bool flipY)
{
	// The renderer negates vertex Y when flipY is set; pairing that with bottom=-height keeps canvas y=0 at
	// the top of the screen and y=height at the bottom. Without the flip the plain y-down bounds apply.
	return glm::vec2(flipY ? -height : height, 0.f);
}

int pg::ui::GetElementDepth(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent)
{
	int depth = 0;
	pg::ecs::Entity parentEntity = baseComponent.m_Parent;
	while (parentEntity != pg::ecs::null && accessor.AnyOf<pg::ui::BaseComponent>(parentEntity))
	{
		depth += 1;
		parentEntity = accessor.Get<const pg::ui::BaseComponent>(parentEntity).m_Parent;
	}
	return depth;
}

bool pg::ui::IsUIElementEnabled(pg::CheckedRegistryAccessor& accessor, const pg::ui::BaseComponent& baseComponent)
{
	bool enabled = baseComponent.m_Enabled;
	pg::ecs::Entity parentEntity = baseComponent.m_Parent;
	while (enabled && parentEntity != pg::ecs::null)
	{
		PG_CORE_EXCEPT(accessor.AnyOf<pg::ui::BaseComponent>(parentEntity), "parent entity does not have base component");
		const pg::ui::BaseComponent& parentComponent = accessor.Get<const pg::ui::BaseComponent>(parentEntity);
		enabled = parentComponent.m_Enabled;
		parentEntity = parentComponent.m_Parent;
	}
	return enabled;
}

std::vector<pg::ecs::Entity> pg::ui::GetUIChildrenForElement(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent)
{
	std::vector<pg::ecs::Entity> children{};
	auto view = accessor.View<const pg::ui::BaseComponent>();
	for (auto child : view)
	{
		const pg::ui::BaseComponent& baseComponent = view.get<const pg::ui::BaseComponent>(child);
		if (baseComponent.m_Parent == ent)
			children.push_back(child);
	}
	return children;
}
