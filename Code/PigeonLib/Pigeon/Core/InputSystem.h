#pragma once
#include "Pigeon/ECS/System.h"

namespace pg
{
	class InputSystem : public pg::System
	{
	public:
		InputSystem() = default;
		~InputSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
