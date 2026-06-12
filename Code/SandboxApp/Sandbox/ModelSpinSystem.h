#pragma once
#include "Pigeon/ECS/System.h"

namespace sbx
{
	class ModelSpinSystem : public pg::System
	{
	public:
		ModelSpinSystem() = default;
		~ModelSpinSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
