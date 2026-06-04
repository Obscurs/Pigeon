#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	class CameraSystem : public pg::System
	{
	public:
		CameraSystem() = default;
		~CameraSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	private:
	};
}