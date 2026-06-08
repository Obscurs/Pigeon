#pragma once
#include "Pigeon/ECS/System.h"

namespace sbx
{
	class TransformResolveSystem : public pg::System
	{
	public:
		TransformResolveSystem() = default;
		~TransformResolveSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
