#pragma once
#include "Pigeon/Core/UUID.h"

namespace pig
{
	struct EngineConfigSingletonComponent
	{
		EngineConfigSingletonComponent() {};
		EngineConfigSingletonComponent(const EngineConfigSingletonComponent&) = default;

		pig::UUID m_DefaultQuadShaderID;
		pig::UUID m_DefaultTextShaderID;
		pig::UUID m_DefaultFontID;
	};
}