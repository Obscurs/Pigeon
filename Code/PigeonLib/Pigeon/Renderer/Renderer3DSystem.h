#pragma once
#include "Pigeon/ECS/System.h"

namespace pg
{
	class Renderer3DSystem : public pg::System
	{
	public:
		Renderer3DSystem() = default;
		~Renderer3DSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
