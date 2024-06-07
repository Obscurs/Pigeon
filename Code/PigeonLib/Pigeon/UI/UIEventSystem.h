#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig::ui
{
	class UIEventSystem : public pig::System
	{
	public:
		UIEventSystem();
		~UIEventSystem() = default;
		void Update(float dt) override;
	private:
		void CleanOneFrameComponents();
	};
}