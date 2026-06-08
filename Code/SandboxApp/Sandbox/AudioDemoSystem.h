#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class AudioDemoSystem : public pg::System
	{
	public:
		AudioDemoSystem() = default;
		~AudioDemoSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
