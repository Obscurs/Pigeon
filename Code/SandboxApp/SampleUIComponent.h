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

		pg::OrthographicCameraController m_CameraController;

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

		pg::S_Ptr<pg::Texture2D> m_Texture;

		pg::UUID m_TextureID1{};
		pg::S_Ptr<pg::Font> m_Font;

		pg::ecs::Entity m_UIEntity1{ pg::ecs::null };
		pg::ecs::Entity m_UIEntity2{ pg::ecs::null };
	};
}
