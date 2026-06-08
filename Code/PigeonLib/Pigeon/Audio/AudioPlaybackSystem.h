#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	class AudioPlaybackSystem : public pg::System
	{
	public:
		AudioPlaybackSystem() = default;
		~AudioPlaybackSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
