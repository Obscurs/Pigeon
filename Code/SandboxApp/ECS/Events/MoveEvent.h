#pragma once
#include <entt/entt.hpp>

namespace sbx
{
	struct MoveEvent
	{
		int amountX;
		int amountY;
		entt::entity entity;
	};
}
