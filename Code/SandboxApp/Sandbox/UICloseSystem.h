#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class UICloseSystem : public pg::System
	{
	public:
		UICloseSystem() = default;
		~UICloseSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
