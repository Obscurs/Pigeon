#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class SaveDataDebugPanelSystem : public pg::System
	{
	public:
		SaveDataDebugPanelSystem() = default;
		~SaveDataDebugPanelSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
