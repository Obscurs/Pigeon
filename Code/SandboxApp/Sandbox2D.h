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
		~Sandbox2D();

		void OnUpdate(pig::Timestep ts) override;

		virtual void OnImGuiRender() override;
		void OnEvent(pig::Event& event) override;

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		pig::U_Ptr<pig::VertexBuffer> m_VertexBuffer = nullptr;
		pig::U_Ptr<pig::IndexBuffer> m_IndexBuffer = nullptr;

		pig::S_Ptr<pig::Shader> m_Shader = nullptr;

		//ARNAU TODO? spritesheet
		//pig::U_Ptr<pig::Texture2D> m_Texture; 

		pig::OrthographicCameraController m_CameraController;

		glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

		SceneData m_SceneData;
	};
}

