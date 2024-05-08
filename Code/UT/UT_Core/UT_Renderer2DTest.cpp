#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Utils/TestApp.h"

#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/MSDFData.h"
#include <Pigeon/Renderer/OrthographicCamera.h>
#include <Pigeon/Renderer/Renderer2D.h>
#include <Pigeon/Renderer/Sprite.h>
#include <Pigeon/Renderer/Texture.h>

#include <Platform/Testing/TestingHelper.h>
#include <Platform/Testing/TestingTexture.h>

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
			CHECK(data.m_QuadShader);
			CHECK(data.m_TextShader);
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
			pig::Renderer2D::AddTexture("Assets/Test/SampleTexture.png", "SampleTexture", pig::EMappedTextureType::eQuad);

			pig::Renderer2D::BeginScene(cameraController);

			glm::vec3 pos(4.f, 5.f, 6.f);
			glm::vec3 col(7.f, 8.f, 9.f);
			glm::vec3 scale(1.f, 2.f, 3.f);
			pig::Renderer2D::DrawQuad(pos, scale, "SampleTexture");

			pig::Renderer2D::EndScene();
		}
		SECTION("Draw sprite")
		{
			std::string textureName("SampleTexture1");
			const int textureHeight = 1024;
			const int textureWidth = 1024;
			pig::TestingTexture2D::s_ExpectedWidth = textureWidth;
			pig::TestingTexture2D::s_ExpectedHeight = textureHeight;
			pig::Renderer2D::AddTexture("Assets/Test/SampleTexture.png", textureName, pig::EMappedTextureType::eQuad);

			const glm::vec3 pos(4.f, 5.f, 6.f);
			const glm::vec2 scale(1.f, 2.f);
			const glm::vec2 spriteSize(128, 256);
			const glm::vec2 offset(256, 512);

			glm::vec2 offsetNormalized(offset.x / textureWidth, offset.y / textureHeight);
			glm::vec2 sizeNormalized(spriteSize.x / textureWidth, spriteSize.y / textureHeight);

			const pig::S_Ptr<pig::Texture2D> texture = pig::Renderer2D::GetTexture(textureName);
			glm::vec4 texCoordsRect = texture->GetTexCoordsRect(offset, spriteSize);
			
			pig::Sprite sprite(pos, scale, texCoordsRect, textureName);
			const pig::Sprite::Data& spriteData = sprite.GetData();
			CHECK(spriteData.m_Position == pos);
			CHECK(spriteData.m_TextureSize == glm::vec2(textureWidth, textureHeight));
			CHECK(sprite.GetTextureSize() == glm::vec2(textureWidth, textureHeight));
			CHECK(spriteData.m_Scale == scale);
			CHECK(spriteData.m_TexCoordsRect == texCoordsRect);
			CHECK(spriteData.m_TextureID == textureName);
			CHECK(sprite.GetTextureID() == textureName);
			CHECK(sprite.GetTexCoordsRect() == texCoordsRect);

			const glm::vec3 pos2(1.2f, 2.3f, 3.4f);
			sprite.SetPosition(pos2);
			CHECK(spriteData.m_Position == pos2);
			CHECK(sprite.GetPosition() == pos2);

			const glm::vec2 scale2(4.2f, 5.3f);
			sprite.SetScale(scale2);
			CHECK(spriteData.m_Scale == scale2);
			CHECK(sprite.GetScale() == scale2);

			//Actual sprite draw
			pig::Renderer2D::BeginScene(cameraController);
			pig::Renderer2D::DrawSprite(sprite);
			pig::Renderer2D::EndScene();

			REQUIRE(pig::TestingHelper::GetInstance().m_VertexBufferSetVertices.size() == 1);
			const pig::TestingHelper::VertexBufferSetVerticesData& interfaceData = pig::TestingHelper::GetInstance().m_VertexBufferSetVertices[0];
			CHECK(interfaceData.m_Count == 4);
			CHECK(interfaceData.m_CountOffset == 0);

			const int strideOffset = 10;

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[7 + strideOffset * 0] == offsetNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[8 + strideOffset * 0] == offsetNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[7 + strideOffset * 1] == offsetNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[8 + strideOffset * 1] == offsetNormalized.y + sizeNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[7 + strideOffset * 2] == offsetNormalized.x + sizeNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[8 + strideOffset * 2] == offsetNormalized.y + sizeNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[7 + strideOffset * 3] == offsetNormalized.x + sizeNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[8 + strideOffset * 3] == offsetNormalized.y);
		}
		SECTION("Draw Text")
		{
			pig::Application& app = pig::CreateApplication();

			pig::OrthographicCameraController cameraController(1280.0f / 720.0f);
			const pig::Renderer2D::Data& data = pig::Renderer2D::GetData();

			glm::vec3 pos(1.f, 2.f, 3.f);
			glm::vec3 col(11.f, 22.f, 33.f);
			glm::vec3 scale(4.f, 5.f, 6.f);
			glm::vec2 textConfig(0.5f, 1.5f);

			pig::S_Ptr<pig::Font> testFont = std::make_shared<pig::Font>("Assets/Test/OpenSans-Regular.ttf");
			const std::string testFontId = testFont->GetFontID();
			CHECK(testFontId == "Assets/Test/OpenSans-Regular.ttf");
			CHECK(testFont->GetMSDFData()->Glyphs.size() == 191);
			CHECK(FLOAT_EQ(testFont->GetMSDFData()->FontGeometry.getMetrics().lineHeight, 1.3618164062f));

			const std::string testText("Sample\nText\n1");
			glm::mat4 stringTransform = glm::mat4(1.0f); // Identity matrix
			stringTransform = glm::translate(stringTransform, pos); // Apply translation
			stringTransform = glm::scale(stringTransform, scale); // Apply scaling

			pig::Renderer2D::BeginScene(cameraController);
			pig::Renderer2D::DrawString(testText, testFont, stringTransform, glm::vec4(col, 1.0f), textConfig.x, textConfig.y);
			pig::Renderer2D::EndScene();
			//TODO Arnau finish this UT
			/*
			REQUIRE(pig::TestingHelper::GetInstance().m_VertexBufferSetVertices.size() == 1);
			const pig::TestingHelper::VertexBufferSetVerticesData& interfaceData = pig::TestingHelper::GetInstance().m_VertexBufferSetVertices[0];
			CHECK(interfaceData.m_Count == 4);
			CHECK(interfaceData.m_CountOffset == 0);

			const int strideOffset = 10;

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[7 + strideOffset * 0] == offsetNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[8 + strideOffset * 0] == offsetNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[7 + strideOffset * 1] == offsetNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[8 + strideOffset * 1] == offsetNormalized.y + sizeNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[7 + strideOffset * 2] == offsetNormalized.x + sizeNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[8 + strideOffset * 2] == offsetNormalized.y + sizeNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[7 + strideOffset * 3] == offsetNormalized.x + sizeNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[8 + strideOffset * 3] == offsetNormalized.y);
			*/
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

		glm::vec3 pos4(1.f, 2.f, 3.f);
		glm::vec3 col4(11.f, 22.f, 33.f);
		glm::vec3 scale4(4.f, 5.f, 6.f);
		glm::vec2 textConfig(0.5f, 1.5f);

		const unsigned int texWidht = 128;
		const unsigned int texHeight = 64;
		const unsigned int texChannels = 3;
		std::vector<unsigned char> texData(texWidht * texHeight * texChannels, 255);

		CHECK(data.m_TextureMap.size() == 1);
		pig::TestingTexture2D::s_ExpectedWidth = 1024;
		pig::TestingTexture2D::s_ExpectedHeight = 1024;
		pig::Renderer2D::AddTexture("Assets/Test/SampleTexture.png", "SampleTexture1", pig::EMappedTextureType::eQuad);
		pig::TestingTexture2D::s_ExpectedWidth = 980;
		pig::TestingTexture2D::s_ExpectedHeight = 725;
		pig::Renderer2D::AddTexture("Assets/Test/SampleTexture2.png", "SampleTexture2", pig::EMappedTextureType::eQuad);
		pig::TestingTexture2D::s_ExpectedWidth = 128;
		pig::TestingTexture2D::s_ExpectedHeight = 64;
		pig::Renderer2D::AddTexture(texWidht, texHeight, texChannels, texData.data(), "SampleTexture3", pig::EMappedTextureType::eQuad);

		pig::S_Ptr<pig::Font> testFont = std::make_shared<pig::Font>("Assets/Test/OpenSans-Regular.ttf");
		const std::string testFontId = testFont->GetFontID();

		const pig::S_Ptr<pig::Texture2D> tex1 = pig::Renderer2D::GetTexture("SampleTexture1");
		const pig::S_Ptr<pig::Texture2D> tex2 = pig::Renderer2D::GetTexture("SampleTexture2");
		const pig::S_Ptr<pig::Texture2D> tex3 = pig::Renderer2D::GetTexture("SampleTexture3");
		const pig::S_Ptr<pig::Texture2D> tex4 = pig::Renderer2D::GetTexture(testFontId);
		REQUIRE(tex1);
		CHECK(tex1->GetWidth() == 1024);
		CHECK(tex1->GetHeight() == 1024);
		REQUIRE(tex2);
		CHECK(tex2->GetWidth() == 980);
		CHECK(tex2->GetHeight() == 725);
		REQUIRE(tex3);
		CHECK(tex3->GetWidth() == 128);
		CHECK(tex3->GetHeight() == 64);

		CHECK(data.m_TextureMap.size() == 5);
		const auto it = data.m_TextureMap.find("SampleTexture2");
		CHECK(it != data.m_TextureMap.end());
		CHECK(it->second.m_TextureType == pig::EMappedTextureType::eQuad);

		const auto it2 = data.m_TextureMap.find(testFontId);
		CHECK(it2 != data.m_TextureMap.end());
		CHECK(it2->second.m_TextureType == pig::EMappedTextureType::eText);

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

		const std::string testText("Sample\nText\n1");
		glm::mat4 stringTransform = glm::mat4(1.0f); // Identity matrix
		stringTransform = glm::translate(stringTransform, pos4); // Apply translation
		stringTransform = glm::scale(stringTransform, scale4); // Apply scaling
		pig::Renderer2D::DrawString(testText, testFont, stringTransform, glm::vec4(col4, 1.0f), textConfig.x, textConfig.y);

		SECTION("Multiple textures")
		{
			REQUIRE(data.m_BatchMap.find("") != data.m_BatchMap.end());
			REQUIRE(data.m_BatchMap.find("SampleTexture1") != data.m_BatchMap.end());
			REQUIRE(data.m_BatchMap.find("SampleTexture2") != data.m_BatchMap.end());
			REQUIRE(data.m_BatchMap.find("SampleTexture3") != data.m_BatchMap.end());
			REQUIRE(data.m_BatchMap.find(testFontId) != data.m_BatchMap.end());
			const pig::Renderer2D::Data::BatchData& texBatch1 = data.m_BatchMap.at("");
			const pig::Renderer2D::Data::BatchData& texBatch2 = data.m_BatchMap.at("SampleTexture1");
			const pig::Renderer2D::Data::BatchData& texBatch3 = data.m_BatchMap.at("SampleTexture2");
			const pig::Renderer2D::Data::BatchData& texBatch4 = data.m_BatchMap.at("SampleTexture3");
			const pig::Renderer2D::Data::BatchData& texBatch5 = data.m_BatchMap.at(testFontId);

			CHECK(texBatch1.m_IndexCount == 18);
			CHECK(texBatch1.m_VertexCount == 12);
			CHECK(texBatch2.m_IndexCount == 18);
			CHECK(texBatch2.m_VertexCount == 12);
			CHECK(texBatch3.m_IndexCount == 18);
			CHECK(texBatch3.m_VertexCount == 12);
			CHECK(texBatch4.m_IndexCount == 12);
			CHECK(texBatch4.m_VertexCount == 8);
			CHECK(texBatch5.m_IndexCount == 66); //11 characters * 6 vertices (2 triangles)
			CHECK(texBatch5.m_VertexCount == 44); // 11 characters * 4 vertices

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

