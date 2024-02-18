#pragma once
#include <catch.hpp>
#include <cstdlib>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Utils/TestApp.h"

#include <Pigeon/Renderer/OrthographicCamera.h>
#include <Pigeon/Renderer/Renderer2D.h>
#include <Pigeon/Renderer/Texture.h>

#include <Platform/DirectX11/Dx11Buffer.h>
#include <Platform/DirectX11/Dx11Context.h>
#include <Platform/DirectX11/Dx11RendererAPI.h>
#include <Platform/DirectX11/Dx11Shader.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace CatchTestsetFail
{
	TEST_CASE("Core.Renderer2D::Draw")
	{
		pig::Application& app = pig::CreateApplication();

		pig::OrthographicCameraController cameraController(1280.0f / 720.0f);

		SECTION("Empty call")
		{
			const pig::Renderer2D::Data& data = pig::Renderer2D::GetData();
			CHECK(!data.m_Camera);
			CHECK(data.m_Shader);
			CHECK(data.m_VertexBuffer);
			CHECK(data.m_IndexBuffer);
			CHECK(data.m_TextureMap.size() == 1);

			pig::Renderer2D::Clear({ 0.f, 0.f, 0.f, 1.f });
			pig::Renderer2D::BeginScene(cameraController);
			CHECK(data.m_Camera);
			pig::Renderer2D::EndScene();
			CHECK(!data.m_Camera);
		}
		SECTION("Draw quad")
		{
			pig::Renderer2D::BeginScene(cameraController);

			glm::vec3 pos(4.f, 5.f, 6.f);
			glm::vec3 col(7.f, 8.f, 9.f);
			glm::vec3 scale(1.f, 2.f, 3.f);
			pig::Renderer2D::DrawQuad(pos, scale, col);

			pig::Renderer2D::EndScene();
		}
		SECTION("Draw textured quad")
		{
			pig::Renderer2D::AddTexture("Assets/Test/SampleTexture.png", "SampleTexture");

			pig::Renderer2D::BeginScene(cameraController);

			glm::vec3 pos(4.f, 5.f, 6.f);
			glm::vec3 col(7.f, 8.f, 9.f);
			glm::vec3 scale(1.f, 2.f, 3.f);
			pig::Renderer2D::DrawQuad(pos, scale, "SampleTexture");

			pig::Renderer2D::EndScene();
		}
	}
	TEST_CASE("Core.Renderer2D::BatchRendering")
	{
		pig::Application& app = pig::CreateApplication();

		pig::OrthographicCameraController cameraController(1280.0f / 720.0f);
		const pig::Renderer2D::Data& data = pig::Renderer2D::GetData();

		glm::vec3 pos1(4.f, 5.f, 6.f);
		glm::vec3 col1(7.f, 8.f, 9.f);
		glm::vec3 scale1(1.f, 2.f, 3.f);

		glm::vec3 pos2(1.f, 2.f, 1.f);
		glm::vec3 col2(4.f, 2.f, 1.f);
		glm::vec3 scale2(2.f, 1.f, 2.f);

		glm::vec3 pos3(4.f, 6.f, 1.f);
		glm::vec3 col3(2.f, 3.f, 4.f);
		glm::vec3 scale3(8.f, 1.f, 2.f);

		const unsigned int texWidht = 128;
		const unsigned int texHeight = 64;
		const unsigned int texChannels = 3;
		std::vector<unsigned char> texData(texWidht * texHeight * texChannels, 255);

		CHECK(data.m_TextureMap.size() == 1);
		pig::Renderer2D::AddTexture("Assets/Test/SampleTexture.png", "SampleTexture1");
		pig::Renderer2D::AddTexture("Assets/Test/SampleTexture2.png", "SampleTexture2");
		pig::Renderer2D::AddTexture(texWidht, texHeight, texChannels, texData.data(), "SampleTexture3");
		CHECK(data.m_TextureMap.size() == 4);
		CHECK(data.m_TextureMap.find("SampleTexture2") != data.m_TextureMap.end());

		pig::Renderer2D::Clear({ 0.f, 0.f, 0.f, 1.f });
		pig::Renderer2D::BeginScene(cameraController);

		pig::Renderer2D::DrawQuad(pos1, scale1, col1);
		pig::Renderer2D::DrawQuad(pos1, scale1, "SampleTexture1");
		pig::Renderer2D::DrawQuad(pos2, scale2, "SampleTexture1");
		pig::Renderer2D::DrawQuad(pos2, scale2, col2);
		pig::Renderer2D::DrawQuad(pos1, scale1, "SampleTexture2");
		pig::Renderer2D::DrawQuad(pos3, scale3, "SampleTexture2");
		pig::Renderer2D::DrawQuad(pos3, scale3, "SampleTexture1");
		pig::Renderer2D::DrawQuad(pos1, scale1, "SampleTexture3");
		pig::Renderer2D::DrawQuad(pos2, scale1, "SampleTexture3");
		pig::Renderer2D::DrawQuad(pos1, scale3, "SampleTexture2");
		pig::Renderer2D::DrawQuad(pos3, scale3, col3);

		SECTION("Multiple textures")
		{
			REQUIRE(data.m_BatchMap.find("") != data.m_BatchMap.end());
			REQUIRE(data.m_BatchMap.find("SampleTexture1") != data.m_BatchMap.end());
			REQUIRE(data.m_BatchMap.find("SampleTexture2") != data.m_BatchMap.end());
			REQUIRE(data.m_BatchMap.find("SampleTexture3") != data.m_BatchMap.end());
			const pig::Renderer2D::Data::BatchData& texBatch1 = data.m_BatchMap.at("");
			const pig::Renderer2D::Data::BatchData& texBatch2 = data.m_BatchMap.at("SampleTexture1");
			const pig::Renderer2D::Data::BatchData& texBatch3 = data.m_BatchMap.at("SampleTexture2");
			const pig::Renderer2D::Data::BatchData& texBatch4 = data.m_BatchMap.at("SampleTexture3");

			CHECK(texBatch1.m_IndexCount == 18);
			CHECK(texBatch1.m_VertexCount == 12);
			CHECK(texBatch2.m_IndexCount == 18);
			CHECK(texBatch2.m_VertexCount == 12);
			CHECK(texBatch3.m_IndexCount == 18);
			CHECK(texBatch3.m_VertexCount == 12);
			CHECK(texBatch4.m_IndexCount == 12);
			CHECK(texBatch4.m_VertexCount == 8);

			CHECK(texBatch1.m_VertexBuffer[0] == 3.5f);
			CHECK(texBatch1.m_IndexBuffer[0] == 0.f);
			CHECK(texBatch2.m_VertexBuffer[3] == 1.f);
			CHECK(texBatch2.m_IndexBuffer[5] == 0.f);
			CHECK(texBatch3.m_VertexBuffer[4] == 1.f);
			CHECK(texBatch3.m_IndexBuffer[6] == 4.f);
			CHECK(texBatch4.m_VertexBuffer[7] == 0.f);
			CHECK(texBatch4.m_IndexBuffer[11] == 4.f);
		}
		SECTION("Going over max count")
		{
			for (unsigned int i = 3; i < pig::Renderer2D::BATCH_MAX_COUNT; ++i)
			{
				pig::Renderer2D::DrawQuad(pos1, scale1, col1);
			}

			{
				REQUIRE(data.m_BatchMap.find("") != data.m_BatchMap.end());
				REQUIRE(data.m_BatchMap.find("SampleTexture1") != data.m_BatchMap.end());
				REQUIRE(data.m_BatchMap.find("SampleTexture2") != data.m_BatchMap.end());
				REQUIRE(data.m_BatchMap.find("SampleTexture3") != data.m_BatchMap.end());
				const pig::Renderer2D::Data::BatchData& texBatch1 = data.m_BatchMap.at("");
				const pig::Renderer2D::Data::BatchData& texBatch2 = data.m_BatchMap.at("SampleTexture1");
				const pig::Renderer2D::Data::BatchData& texBatch3 = data.m_BatchMap.at("SampleTexture2");
				const pig::Renderer2D::Data::BatchData& texBatch4 = data.m_BatchMap.at("SampleTexture3");
				CHECK(texBatch1.m_IndexCount == pig::Renderer2D::BATCH_MAX_COUNT * pig::Renderer2D::QUAD_INDEX_COUNT);
				CHECK(texBatch1.m_VertexCount == pig::Renderer2D::BATCH_MAX_COUNT * pig::Renderer2D::QUAD_VERTEX_COUNT);
				CHECK(texBatch2.m_IndexCount == 18);
				CHECK(texBatch2.m_VertexCount == 12);
				CHECK(texBatch3.m_IndexCount == 18);
				CHECK(texBatch3.m_VertexCount == 12);
				CHECK(texBatch4.m_IndexCount == 12);
				CHECK(texBatch4.m_VertexCount == 8);
			}

			pig::Renderer2D::DrawQuad(pos3, scale3, "SampleTexture1");

			{
				REQUIRE(data.m_BatchMap.find("") != data.m_BatchMap.end());
				REQUIRE(data.m_BatchMap.find("SampleTexture1") != data.m_BatchMap.end());
				REQUIRE(data.m_BatchMap.find("SampleTexture2") != data.m_BatchMap.end());
				REQUIRE(data.m_BatchMap.find("SampleTexture3") != data.m_BatchMap.end());
				const pig::Renderer2D::Data::BatchData& texBatch1 = data.m_BatchMap.at("");
				const pig::Renderer2D::Data::BatchData& texBatch2 = data.m_BatchMap.at("SampleTexture1");
				const pig::Renderer2D::Data::BatchData& texBatch3 = data.m_BatchMap.at("SampleTexture2");
				const pig::Renderer2D::Data::BatchData& texBatch4 = data.m_BatchMap.at("SampleTexture3");
				CHECK(texBatch1.m_IndexCount == pig::Renderer2D::BATCH_MAX_COUNT * pig::Renderer2D::QUAD_INDEX_COUNT);
				CHECK(texBatch1.m_VertexCount == pig::Renderer2D::BATCH_MAX_COUNT * pig::Renderer2D::QUAD_VERTEX_COUNT);
				CHECK(texBatch2.m_IndexCount == 24);
				CHECK(texBatch2.m_VertexCount == 16);
				CHECK(texBatch3.m_IndexCount == 18);
				CHECK(texBatch3.m_VertexCount == 12);
				CHECK(texBatch4.m_IndexCount == 12);
				CHECK(texBatch4.m_VertexCount == 8);
			}

			pig::Renderer2D::DrawQuad(pos1, scale1, col1);

			{
				REQUIRE(data.m_BatchMap.find("") != data.m_BatchMap.end());
				CHECK(data.m_BatchMap.find("SampleTexture1") == data.m_BatchMap.end());
				REQUIRE(data.m_BatchMap.find("SampleTexture2") == data.m_BatchMap.end());
				REQUIRE(data.m_BatchMap.find("SampleTexture3") == data.m_BatchMap.end());
				const pig::Renderer2D::Data::BatchData& texBatch1 = data.m_BatchMap.at("");

				CHECK(texBatch1.m_IndexCount == 6);
				CHECK(texBatch1.m_VertexCount == 4);
			}
		}

		pig::Renderer2D::EndScene();
	}
} // End namespace: CatchTestsetFail

