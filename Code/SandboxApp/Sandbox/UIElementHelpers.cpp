#include "Sandbox/UIElementHelpers.h"

#include "Pigeon/Core/UUID.h"
#include "Pigeon/ECS/CheckedRegistryAccessor.h"
#include "Pigeon/UI/UIComponents.h"

namespace
{
	template<typename EventComponent>
	bool HasElementEvent(pg::CheckedRegistryAccessor& accessor, const pg::UUID& elementID)
	{
		auto view = accessor.View<const EventComponent>();
		for (pg::ecs::Entity ent : view)
		{
			if (view.get<const EventComponent>(ent).m_ElementID == elementID)
			{
				return true;
			}
		}
		return false;
	}
}

pg::ecs::Entity sbx::FindUIElementByUUID(pg::CheckedRegistryAccessor& accessor, const pg::UUID& uuid)
{
	auto view = accessor.View<const pg::ui::BaseComponent>();
	for (pg::ecs::Entity ent : view)
	{
		if (view.get<const pg::ui::BaseComponent>(ent).m_UUID == uuid)
		{
			return ent;
		}
	}
	return pg::ecs::null;
}

bool sbx::IsElementClicked(pg::CheckedRegistryAccessor& accessor, const pg::UUID& elementID)
{
	return HasElementEvent<pg::ui::UIOnClickOneFrameComponent>(accessor, elementID);
}

bool sbx::IsElementHovered(pg::CheckedRegistryAccessor& accessor, const pg::UUID& elementID)
{
	return HasElementEvent<pg::ui::UIOnHoverOneFrameComponent>(accessor, elementID);
}

bool sbx::IsElementReleased(pg::CheckedRegistryAccessor& accessor, const pg::UUID& elementID)
{
	return HasElementEvent<pg::ui::UIOnReleaseOneFrameComponent>(accessor, elementID);
}
