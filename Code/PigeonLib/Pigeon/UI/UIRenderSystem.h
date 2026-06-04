#pragma once
#include "Pigeon/ECS/System.h"

namespace pg::ui
{
	class UIRenderSystem : public pg::System
	{
	public:
		UIRenderSystem() = default;
		~UIRenderSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}