#pragma once
#include <typeindex>
#include <unordered_set>

#include "Pigeon/Core/Timestep.h"

namespace pg
{
	struct SystemAccessDecl
	{
		std::unordered_set<std::type_index> readSet;
		std::unordered_set<std::type_index> writeSet;
		std::unordered_set<std::type_index> addSet;
		std::unordered_set<std::type_index> inframeAddSet;
	};

	class System
	{
	public:
		System() = default;
		~System() = default;
		virtual void Update(const pg::Timestep& ts) = 0;

		// Systems that do not override this will assert on any component access.
		// Every concrete system must override DeclareAccess() and declare its readSet, writeSet, and addSet.
		virtual SystemAccessDecl DeclareAccess() const
		{
			return SystemAccessDecl{};
		}
	};
}