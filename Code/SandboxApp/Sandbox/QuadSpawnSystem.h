#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class QuadSpawnSystem : public pg::System
	{
	public:
		QuadSpawnSystem() = default;
		~QuadSpawnSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
