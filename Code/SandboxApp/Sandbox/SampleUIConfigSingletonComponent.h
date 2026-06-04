#pragma once
#include "Pigeon/Core/UUID.h"

namespace sbx
{
	struct SampleUIConfigSingletonComponent
	{
		SampleUIConfigSingletonComponent() {};
		SampleUIConfigSingletonComponent(const SampleUIConfigSingletonComponent&) = default;

		pg::UUID m_UUIDUI1;
		pg::UUID m_UUIDUI2;
		pg::UUID m_DefaultFontID;
		pg::UUID m_MainLayoutID;
	};
}