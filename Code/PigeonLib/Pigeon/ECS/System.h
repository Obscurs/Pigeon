#pragma once
#include <entt/entt.hpp>
#include <typeindex>
#include <unordered_set>

#include "Pigeon/Core/Timestep.h"

namespace pig
{
	struct SystemAccessDecl
	{
		std::unordered_set<std::type_index> readSet;
		std::unordered_set<std::type_index> writeSet;
		std::unordered_set<std::type_index> addSet;
	};

	class System
	{
	public:
		System() = default;
		~System() = default;
		virtual void Update(const pig::Timestep& ts) = 0;

		// Systems that do not override this will assert on any component access.
		// Every concrete system must override DeclareAccess() and declare its readSet, writeSet, and addSet.
		virtual SystemAccessDecl DeclareAccess() const
		{
			return SystemAccessDecl{};
		}
	};
}