#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class SceneSetupSystem : public pg::System
	{
	public:
		SceneSetupSystem() = default;
		~SceneSetupSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
