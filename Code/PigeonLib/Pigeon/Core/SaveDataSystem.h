#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	class SaveDataSystem : public pg::System
	{
	public:
		SaveDataSystem() = default;
		~SaveDataSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
