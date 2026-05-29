#pragma once

#include "Pigeon/ECS/System.h"

namespace pig
{
	class ResourceManagerSystem : public pig::System
	{
	public:
		ResourceManagerSystem() = default;
		~ResourceManagerSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	private:
	};
}