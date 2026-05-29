#pragma once

#include "Pigeon/ECS/System.h"

namespace pig
{
	class ConfigLoaderSystem : public pig::System
	{
	public:
		ConfigLoaderSystem() = default;
		~ConfigLoaderSystem() = default;
		void Update(const pig::Timestep& ts) override;
		pig::SystemAccessDecl DeclareAccess() const override;
	private:
	};
}