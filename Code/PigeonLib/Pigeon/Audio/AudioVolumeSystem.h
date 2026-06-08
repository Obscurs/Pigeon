#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	class AudioVolumeSystem : public pg::System
	{
	public:
		AudioVolumeSystem() = default;
		~AudioVolumeSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
