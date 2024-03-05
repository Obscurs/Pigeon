#pragma once

#include "Pigeon/Layer.h"
#include "Pigeon/Renderer/Buffer.h"
#include "Pigeon/OrthographicCameraController.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Texture.h"

namespace sbx
{
	class Sandbox2D : public pig::Layer
	{
	public:
		Sandbox2D();
		~Sandbox2D() = default;

		void OnUpdate(pig::Timestep ts) override;

		virtual void OnImGuiRender() override;
		bool OnEvent(const pig::Event& event) override;

	private:

		//ARNAU TODO? spritesheet
		//pig::U_Ptr<pig::Texture2D> m_Texture; 

		pig::OrthographicCameraController m_CameraController;

		glm::vec3 m_ColorQuad1;
		glm::vec3 m_PosQuad1;
		glm::vec3 m_ScaleQuad1;

		glm::vec3 m_ColorQuad2;
		glm::vec3 m_PosQuad2;
		glm::vec3 m_ScaleQuad2;

		glm::vec3 m_PosText;
		glm::vec3 m_ColorText;

		pig::S_Ptr<pig::Texture2D> m_Texture;
	};
}

