#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class UIButtonSystem : public pg::System
	{
	public:
		UIButtonSystem() = default;
		~UIButtonSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
