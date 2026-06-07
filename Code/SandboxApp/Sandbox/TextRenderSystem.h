#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class TextRenderSystem : public pg::System
	{
	public:
		TextRenderSystem() = default;
		~TextRenderSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
