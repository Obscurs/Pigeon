#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig::ui
{
	class LayoutControlSystem : public pig::System
	{
	public:
		LayoutControlSystem();
		~LayoutControlSystem() = default;
		void Update(float dt) override;
	};
}