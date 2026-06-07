#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class UIStatusSystem : public pg::System
	{
	public:
		UIStatusSystem() = default;
		~UIStatusSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
