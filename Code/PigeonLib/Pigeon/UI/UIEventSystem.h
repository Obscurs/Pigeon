#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig::ui
{
	class UIEventSystem : public pig::System
	{
	public:
		UIEventSystem() = default;
		~UIEventSystem() = default;
		void Update(const pig::Timestep& ts) override;
	private:
		void CleanOneFrameComponents();
	};
}