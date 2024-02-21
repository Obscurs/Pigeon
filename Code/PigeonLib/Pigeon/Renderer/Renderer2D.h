#pragma once

#include "RenderCommand.h"

#include "Pigeon/OrthographicCameraController.h"
#include "Pigeon/Renderer/Buffer.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Sprite.h"
#include "Pigeon/Renderer/Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pig 
{
	class Renderer2D
	{
	public:
		static const unsigned int VERTEX_ATRIB_COUNT = 10;
		static const unsigned int QUAD_VERTEX_COUNT = 4;
		static const unsigned int QUAD_INDEX_COUNT = 6;
		static const unsigned int BATCH_MAX_COUNT = 1000;

		struct Data
		{
			~Data()
			{
				m_VertexBuffer = nullptr;
				m_IndexBuffer = nullptr;
				m_Shader = nullptr;
				m_Camera = nullptr;
				m_TextureMap.clear();
				m_BatchMap.clear();
			}

			struct BatchData
			{
				float m_VertexBuffer[VERTEX_ATRIB_COUNT * QUAD_VERTEX_COUNT * pig::Renderer2D::BATCH_MAX_COUNT];
				uint32_t m_IndexBuffer[QUAD_INDEX_COUNT * pig::Renderer2D::BATCH_MAX_COUNT];

				unsigned int m_VertexCount = 0;
				unsigned int m_IndexCount = 0;
			};

			pig::S_Ptr<pig::VertexBuffer> m_VertexBuffer = nullptr;
			pig::S_Ptr<pig::IndexBuffer> m_IndexBuffer = nullptr;

			pig::S_Ptr<pig::Shader> m_Shader = nullptr;

			const pig::OrthographicCameraController* m_Camera = nullptr;
			
			std::unordered_map<std::string, pig::S_Ptr<pig::Texture2D>> m_TextureMap;
			std::unordered_map<std::string, BatchData> m_BatchMap;
		};

		static void Destroy();

		static void Init();

		static void Clear(const glm::vec4& color);

		static void BeginScene(const pig::OrthographicCameraController& cameraController);
		static void EndScene();

		static const pig::Texture2D* GetTexture(const std::string& handle);
		static void AddTexture(const std::string& path, const std::string& handle);
		static void AddTexture(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data, const std::string& handle);

		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col);
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const std::string& handle);

		static void DrawSprite(const pig::Sprite& sprite);

#ifdef TESTS_ENABLED
		static const Data& GetData() { return s_Data; }
#endif
	private:
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& col, const std::string& handle, glm::vec4 texRect);
		static void Flush();

		static void Submit(unsigned int count);

		static Data s_Data;
	};
}