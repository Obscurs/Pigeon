#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	class ConfigLoaderSystem : public pg::System
	{
	public:
		ConfigLoaderSystem() = default;
		~ConfigLoaderSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	private:
	};
}