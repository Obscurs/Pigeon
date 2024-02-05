#pragma once

#include "RenderCommand.h"

#include "Pigeon/OrthographicCameraController.h"
#include "Pigeon/Renderer/Buffer.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pig 
{
	class Renderer2D
	{
	public:
		struct Data
		{
			pig::U_Ptr<pig::VertexBuffer> m_VertexBuffer = nullptr;
			pig::U_Ptr<pig::IndexBuffer> m_IndexBuffer = nullptr;
			pig::S_Ptr<pig::Shader> m_Shader = nullptr;
			pig::U_Ptr<pig::Texture2D> m_WhiteTexture = nullptr;

			const pig::OrthographicCameraController* m_Camera = nullptr;
		};

		static void Init();

		static void Clear(const glm::vec4& color);

		static void BeginScene(const pig::OrthographicCameraController& cameraController);
		static void EndScene();

		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col);
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const pig::Texture2D& texture);

#ifdef TESTS_ENABLED
		static const Data& GetData() { return s_Data; }
#endif
	private:
		static void Submit(unsigned int count);

		static Data s_Data;
	};
}