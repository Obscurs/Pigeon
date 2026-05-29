#pragma once
#include "Pigeon/ECS/System.h"

namespace pig::ui
{
	class UIRenderSystem : public pig::System
	{
	public:
		UIRenderSystem() = default;
		~UIRenderSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	};
}