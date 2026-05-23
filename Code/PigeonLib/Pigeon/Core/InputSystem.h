#pragma once
#include "Pigeon/ECS/System.h"

namespace pig
{
	class InputSystem : public pig::System
	{
	public:
		InputSystem() = default;
		~InputSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	};
}
