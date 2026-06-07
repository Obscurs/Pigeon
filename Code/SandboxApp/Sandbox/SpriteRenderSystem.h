#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class SpriteRenderSystem : public pg::System
	{
	public:
		SpriteRenderSystem() = default;
		~SpriteRenderSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
