#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class CameraControlSystem : public pg::System
	{
	public:
		CameraControlSystem() = default;
		~CameraControlSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
