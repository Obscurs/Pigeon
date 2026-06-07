#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class QuadRenderSystem : public pg::System
	{
	public:
		QuadRenderSystem() = default;
		~QuadRenderSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
