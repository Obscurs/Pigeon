#pragma once
#include "entt.hpp"

namespace pig
{
	//ARNAU TODO change systemType, sandbox can not change this enum so we have to find another way to differentiate systems
	enum class SystemType
	{
		eTest
		, eInput
		, eInvalid
		, eMovement
		, eSprite
	};

	class System
	{
	public:
		System(SystemType type) : m_Type(type) {}
		~System() = default;
		SystemType GetType() { return m_Type; }
		bool IsValid() { return m_Type != SystemType::eInvalid; }
		virtual void Update(float dt) = 0;
	protected:
		SystemType m_Type;
	};
}