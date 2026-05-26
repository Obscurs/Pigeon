#pragma once
#include "Pigeon/Core/Core.h"
#include "Pigeon/Renderer/Buffer.h"
#include "Pigeon/Renderer/OrthographicCamera.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Sprite.h"
#include "Pigeon/Renderer/Texture.h"

namespace pig
{
	static const unsigned int ATRIB_POS_X_INDEX = 0;
	static const unsigned int ATRIB_POS_Y_INDEX = 1;
	static const unsigned int ATRIB_POS_Z_INDEX = 2;
	static const unsigned int ATRIB_COL_R_INDEX = 3;
	static const unsigned int ATRIB_COL_G_INDEX = 4;
	static const unsigned int ATRIB_COL_B_INDEX = 5;
	static const unsigned int ATRIB_COL_A_INDEX = 6;
	static const unsigned int ATRIB_TEX_X_INDEX = 7;
	static const unsigned int ATRIB_TEX_Y_INDEX = 8;
	static const unsigned int ATRIB_TEX_ID_INDEX = 9;

	static const unsigned int VERTEX_ATRIB_COUNT = 10;
	static const unsigned int QUAD_VERTEX_COUNT = 4;
	static const unsigned int QUAD_INDEX_COUNT = 6;
	static const unsigned int BATCH_MAX_COUNT = 1000; //ARNAU TODO: this is hardcoded

	struct MappedTexture
	{
		pig::S_Ptr<pig::Texture2D> m_Texture;
		EMappedTextureType m_TextureType;
	};

	struct RendererDataSingletonComponent
	{
		struct BatchData
		{
			float m_VertexBuffer[VERTEX_ATRIB_COUNT * QUAD_VERTEX_COUNT * BATCH_MAX_COUNT];
			uint32_t m_IndexBuffer[QUAD_INDEX_COUNT * BATCH_MAX_COUNT];

			unsigned int m_VertexCount = 0;
			unsigned int m_IndexCount = 0;
		};

		RendererDataSingletonComponent() {};
		RendererDataSingletonComponent(const RendererDataSingletonComponent&) = default;

		pig::S_Ptr<pig::VertexBuffer> m_VertexBuffer = nullptr;
		pig::S_Ptr<pig::IndexBuffer> m_IndexBuffer = nullptr;

		pig::S_Ptr<pig::Shader> m_QuadShader = nullptr;
		pig::S_Ptr<pig::Shader> m_TextShader = nullptr;

		std::unordered_map<pig::UUID, MappedTexture> m_TextureMap;
		std::unordered_map<pig::UUID, BatchData> m_BatchMap;
		std::unordered_map<int, std::unordered_map<pig::UUID, BatchData>> m_LayerBatchMap;

		pig::UUID m_DefaultTexture = pig::UUID::Generate();
	};
}