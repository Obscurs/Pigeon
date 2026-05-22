#include "Pigeon/Core/UUID.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Texture.h"

namespace sbx
{
	struct SampleUIComponent
	{
		SampleUIComponent() {};
		SampleUIComponent(const SampleUIComponent&) = default;

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

		pig::S_Ptr<pig::Texture2D> m_Texture;

		pig::UUID m_TextureID1{};
		pig::S_Ptr<pig::Font> m_Font;

		entt::entity m_UIEntity1{ entt::null };
		entt::entity m_UIEntity2{ entt::null };
	};
}
