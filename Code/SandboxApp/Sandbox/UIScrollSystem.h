#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	// Drives the configured UI clip panel's scroll offset from the mouse wheel while the pointer hovers it
	// (the engine provides the clip + scroll-offset mutation; this app system feeds it interactive input).
	class UIScrollSystem : public pg::System
	{
	public:
		UIScrollSystem() = default;
		~UIScrollSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
