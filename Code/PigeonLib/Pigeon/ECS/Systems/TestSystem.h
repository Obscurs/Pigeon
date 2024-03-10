#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pig
{
	struct MoveEvent { int amountX; int amountY; entt::entity entity; };
	class TestSystem: public pig::System
	{
	public:
		TestSystem();
		~TestSystem() = default;
		void Update(float dt);
	};
}