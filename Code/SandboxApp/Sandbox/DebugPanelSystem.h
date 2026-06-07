#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class DebugPanelSystem : public pg::System
	{
	public:
		DebugPanelSystem() = default;
		~DebugPanelSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
