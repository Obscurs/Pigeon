#include "Pigeon/Core/UUID.h"

namespace sbx
{
	struct SampleUISingletonComponent
	{
		SampleUISingletonComponent() {};
		SampleUISingletonComponent(const SampleUISingletonComponent&) = default;

		glm::vec3 m_ColorQuad1 { 0.f,0.f,0.f };
		glm::vec3 m_PosQuad1 { 0.f,0.f,0.f };
		glm::vec3 m_ScaleQuad1 { 0.f,0.f,0.f };

		glm::vec3 m_ColorQuad2 { 0.f,0.f,0.f };
		glm::vec3 m_PosQuad2 { 0.f,0.f,0.f };
		glm::vec3 m_ScaleQuad2 { 0.f,0.f,0.f };

		glm::vec3 m_PosText { 0.f,0.f,0.f };
		glm::vec3 m_ScaleText { 0.f,0.f,0.f };
		glm::vec3 m_ColorText { 0.f,0.f,0.f };

		glm::vec3 m_OriginQuad1 { 0.f,0.f,0.f };
		glm::vec3 m_OriginQuad2 { 0.f,0.f,0.f };

		pig::UUID m_FontID;

		pig::UUID m_UUIDUI1;
		pig::UUID m_UUIDUI2;
	};
}
