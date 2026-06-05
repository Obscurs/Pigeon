#pragma once
#include "Pigeon/ECS/System.h"

namespace pg::ui
{
	class UIControlSystem : public pg::System
	{
	public:
		UIControlSystem() = default;
		~UIControlSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}