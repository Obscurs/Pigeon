#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	class ResourceManagerSystem : public pg::System
	{
	public:
		ResourceManagerSystem() = default;
		~ResourceManagerSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	private:
	};
}