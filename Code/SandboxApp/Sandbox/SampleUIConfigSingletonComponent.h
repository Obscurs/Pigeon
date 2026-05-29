#pragma once
#include "Pigeon/Core/UUID.h"

namespace sbx
{
	struct SampleUIConfigSingletonComponent
	{
		SampleUIConfigSingletonComponent() {};
		SampleUIConfigSingletonComponent(const SampleUIConfigSingletonComponent&) = default;

		pig::UUID m_UUIDUI1;
		pig::UUID m_UUIDUI2;
		pig::UUID m_DefaultFontID;
		pig::UUID m_MainLayoutID;
	};
}