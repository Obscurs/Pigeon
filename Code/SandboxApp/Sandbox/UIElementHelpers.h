#pragma once
#include "Pigeon/ECS/Entity.h"

namespace pg
{
	class CheckedRegistryAccessor;
	class UUID;
}

namespace sbx
{
	// Returns the first UI element entity whose BaseComponent UUID matches, or pg::ecs::null when
	// none exists yet (e.g. before the layout has finished loading). The caller's system must
	// declare pg::ui::BaseComponent in its readSet.
	pg::ecs::Entity FindUIElementByUUID(pg::CheckedRegistryAccessor& accessor, const pg::UUID& uuid);

	// True if a UIOnClick / UIOnHover / UIOnRelease one-frame component for the given element id is
	// present this frame. The caller's system must declare the matching event component in its readSet.
	bool IsElementClicked(pg::CheckedRegistryAccessor& accessor, const pg::UUID& elementID);
	bool IsElementHovered(pg::CheckedRegistryAccessor& accessor, const pg::UUID& elementID);
	bool IsElementReleased(pg::CheckedRegistryAccessor& accessor, const pg::UUID& elementID);
}
