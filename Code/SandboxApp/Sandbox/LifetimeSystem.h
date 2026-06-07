#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class LifetimeSystem : public pg::System
	{
	public:
		LifetimeSystem() = default;
		~LifetimeSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
