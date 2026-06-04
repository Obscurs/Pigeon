#pragma once
#include "Pigeon/ECS/System.h"

namespace pg 
{
	class Renderer2DSystem : public pg::System
	{
	public:
		Renderer2DSystem() = default;
		~Renderer2DSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}