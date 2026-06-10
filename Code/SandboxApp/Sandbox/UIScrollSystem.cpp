#include "Sandbox/UIScrollSystem.h"

#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UIElementHelpers.h"

#include <algorithm>

namespace
{
	// Pixels of scroll per wheel notch, and the scrollable range (content overflows the clip downward, so
	// the offset runs from 0 at the top to a negative bottom limit). Tuned for the showcase clip panel.
	constexpr float kScrollSpeed = 20.f;
	constexpr float kMinScrollY = -100.f;
	constexpr float kMaxScrollY = 0.f;

	// True when the pointer is over the panel: some hovered element this frame is the panel itself or one
	// of its descendants (the front-most hover lands on a clipped child, so walk up the parent chain).
	bool IsPointerOverPanel(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity panelEntity)
	{
		auto hoverView = accessor.View<const pg::ui::UIOnHoverOneFrameComponent>();
		for (pg::ecs::Entity hoverEnt : hoverView)
		{
			const pg::UUID hoveredID = hoverView.get<const pg::ui::UIOnHoverOneFrameComponent>(hoverEnt).m_ElementID;
			pg::ecs::Entity element = sbx::FindUIElementByUUID(accessor, hoveredID);
			while (element != pg::ecs::null && accessor.AnyOf<pg::ui::BaseComponent>(element))
			{
				if (element == panelEntity)
				{
					return true;
				}
				element = accessor.Get<const pg::ui::BaseComponent>(element).m_Parent;
			}
		}
		return false;
	}

	float TotalWheelDelta(pg::CheckedRegistryAccessor& accessor)
	{
		float delta = 0.f;
		auto scrollView = accessor.View<const pg::MouseScrolledEventComponent>();
		for (pg::ecs::Entity ent : scrollView)
		{
			delta += scrollView.get<const pg::MouseScrolledEventComponent>(ent).m_YOffset;
		}
		return delta;
	}
}

pg::SystemAccessDecl sbx::UIScrollSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::MouseScrolledEventComponent)),
		std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::UIClipComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::UIUpdateClipOffsetOneFrameComponent)),
	};
	return decl;
}

void sbx::UIScrollSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	if (configView.empty())
	{
		return;
	}
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	const float wheelDelta = TotalWheelDelta(accessor);
	if (wheelDelta == 0.f)
	{
		return;
	}

	pg::ecs::Entity panelEntity = sbx::FindUIElementByUUID(accessor, config.m_ScrollPanelID);
	if (panelEntity == pg::ecs::null || !accessor.AnyOf<pg::ui::UIClipComponent>(panelEntity))
	{
		return;
	}

	if (!IsPointerOverPanel(accessor, panelEntity))
	{
		return;
	}

	const pg::ui::UIClipComponent& clip = accessor.Get<const pg::ui::UIClipComponent>(panelEntity);
	const float scrolledY = std::clamp(clip.m_ScrollOffset.y + wheelDelta * kScrollSpeed, kMinScrollY, kMaxScrollY);

	pg::ui::UIUpdateClipOffsetOneFrameComponent update;
	update.m_ScrollOffset = glm::vec2(clip.m_ScrollOffset.x, scrolledY);
	accessor.EmplaceOneframe<pg::ui::UIUpdateClipOffsetOneFrameComponent>(panelEntity, std::move(update));
}
