#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class QuadAnimationSystem : public pg::System
	{
	public:
		QuadAnimationSystem() = default;
		~QuadAnimationSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
