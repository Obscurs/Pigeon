#pragma once
#include <entt/entt.hpp>
#include "Pigeon/ECS/System.h"

namespace pg::ui
{
	class UIEventSystem : public pg::System
	{
	public:
		UIEventSystem() = default;
		~UIEventSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}