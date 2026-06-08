#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class AudioDebugPanelSystem : public pg::System
	{
	public:
		AudioDebugPanelSystem() = default;
		~AudioDebugPanelSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
