#include "Pigeon/Core/OrthographicCameraController.h"
#include "Pigeon/Core/UUID.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Texture.h"

namespace sbx
{
	struct SampleUIComponent
	{
		SampleUIComponent() = default;
		SampleUIComponent(const SampleUIComponent&) = default;

		pig::OrthographicCameraController m_CameraController;

		glm::vec3 m_ColorQuad1;
		glm::vec3 m_PosQuad1;
		glm::vec3 m_ScaleQuad1;

		glm::vec3 m_ColorQuad2;
		glm::vec3 m_PosQuad2;
		glm::vec3 m_ScaleQuad2;

		glm::vec3 m_PosText;
		glm::vec3 m_ScaleText;
		glm::vec3 m_ColorText;

		glm::vec3 m_OriginQuad1;
		glm::vec3 m_OriginQuad2;

		pig::S_Ptr<pig::Texture2D> m_Texture;

		pig::UUID m_TextureID1{};
		pig::S_Ptr<pig::Font> m_Font;

		entt::entity m_UIEntity1{ entt::null };
		entt::entity m_UIEntity2{ entt::null };
	};
}
