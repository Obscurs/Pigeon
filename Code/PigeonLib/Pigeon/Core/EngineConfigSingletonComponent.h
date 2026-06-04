#pragma once
#include "Pigeon/Core/UUID.h"

namespace pg
{
	struct EngineConfigSingletonComponent
	{
		EngineConfigSingletonComponent() {};
		EngineConfigSingletonComponent(const EngineConfigSingletonComponent&) = default;

		pg::UUID m_DefaultQuadShaderID;
		pg::UUID m_DefaultTextShaderID;
		pg::UUID m_DefaultFontID;
	};
}