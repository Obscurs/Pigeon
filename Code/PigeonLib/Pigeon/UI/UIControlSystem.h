#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig::ui
{
	class UIControlSystem : public pig::System
	{
	public:
		UIControlSystem();
		~UIControlSystem() = default;
		void Update(float dt) override;
	};
}