#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	class SpriteAnimationSystem : public pg::System
	{
	public:
		SpriteAnimationSystem() = default;
		~SpriteAnimationSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
