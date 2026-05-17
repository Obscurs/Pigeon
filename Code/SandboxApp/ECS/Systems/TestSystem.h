#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"
#include "ECS/Events/MoveEvent.h"

namespace sbx
{
	class TestSystem : public pig::System
	{
	public:
		TestSystem();
		~TestSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	};
}
