#pragma once
#include "Pigeon/ECS/System.h"

namespace pg
{
	class TransformComposeSystem : public pg::System
	{
	public:
		TransformComposeSystem() = default;
		~TransformComposeSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
