#pragma once

#include "Pigeon/ECS/System.h"

namespace pig
{
	class CameraSystem : public pig::System
	{
	public:
		CameraSystem() = default;
		~CameraSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	private:
	};
}