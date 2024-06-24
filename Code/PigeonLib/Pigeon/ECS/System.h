#pragma once
#include <entt/entt.hpp>

#include "Pigeon/Core/Timestep.h"

namespace pig
{
	class System
	{
	public:
		System() = default;
		~System() = default;
		virtual void Update(const pig::Timestep& ts) = 0;
	};
}