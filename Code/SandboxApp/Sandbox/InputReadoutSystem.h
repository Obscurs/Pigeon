#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class InputReadoutSystem : public pg::System
	{
	public:
		InputReadoutSystem() = default;
		~InputReadoutSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
