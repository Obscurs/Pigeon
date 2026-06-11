#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class CharacterControlSystem : public pg::System
	{
	public:
		CharacterControlSystem() = default;
		~CharacterControlSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
