#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class UIButtonVisualSystem : public pg::System
	{
	public:
		UIButtonVisualSystem() = default;
		~UIButtonVisualSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
