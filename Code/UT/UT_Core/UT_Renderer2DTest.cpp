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

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-5)

namespace CatchTestsetFail
{
	TEST_CASE("Core.Renderer2D::Draw")
	{
		const glm::ivec2 texcoordsIndex(7, 8);
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

			const glm::vec3 pos(4.f, 5.f, 6.f);
			const glm::vec3 col(7.f, 8.f, 9.f);
			const glm::vec3 scale(1.f, 2.f, 3.f);
			const glm::vec3 origin(4.f, 5.f, 6.f);

			glm::mat4 transform(1.f);
			transform = glm::translate(transform, pos);
			transform = glm::scale(transform, scale);
			pig::Renderer2D::DrawQuad(transform, col, origin);

			pig::Renderer2D::EndScene();
		}
		SECTION("Draw textured quad")
		{
			pig::Renderer2D::AddTexture("Assets/Test/SampleTexture.png", "SampleTexture", pig::EMappedTextureType::eQuad);

			pig::Renderer2D::BeginScene(cameraController);

			const glm::vec3 pos(4.f, 5.f, 6.f);
			const glm::vec3 col(7.f, 8.f, 9.f);
			const glm::vec3 scale(1.f, 2.f, 3.f);
			const glm::vec3 origin(4.f, 5.f, 6.f);

			glm::mat4 transform(1.f);
			transform = glm::translate(transform, pos);
			transform = glm::scale(transform, scale);
			pig::Renderer2D::DrawQuad(transform, "SampleTexture", origin);

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

			const glm::vec3 pos1(4.f, 5.f, 6.f);
			const glm::vec3 scale1(1.f, 2.f, 1.f);
			const glm::vec3 origin1(3.f, 4.f, 5.f);
			const glm::vec2 spriteSize(128, 256);
			const glm::vec2 offset(256, 512);
			const glm::vec4 texcoordsRectTest(0.13f, 1.45f, 44.3f, 0.1f);
			const glm::vec3 pos2(14.f, 34.f, 2.f);
			const glm::vec3 scale2(1.4f, 2.1f, 1.6f);

			glm::mat4 transform1(1.f);
			transform1 = glm::translate(transform1, pos1);
			transform1 = glm::scale(transform1, scale1);

			glm::vec2 offsetNormalized(offset.x / textureWidth, offset.y / textureHeight);
			glm::vec2 sizeNormalized(spriteSize.x / textureWidth, spriteSize.y / textureHeight);

			const pig::S_Ptr<pig::Texture2D> texture = pig::Renderer2D::GetTexture(textureName);
			glm::vec4 texCoordsRect = texture->GetTexCoordsRect(offset, spriteSize);
			
			pig::Sprite sprite(transform1, texCoordsRect, textureName, origin1);
			const pig::Sprite::Data& spriteData = sprite.GetData();

			CHECK(sprite.GetTransform() == transform1);
			CHECK(sprite.GetTextureSize() == glm::vec2(textureWidth, textureHeight));

			CHECK(sprite.GetTextureID() == textureName);
			CHECK(sprite.GetTexCoordsRect() == texCoordsRect);

			sprite.SetTexCoords(texcoordsRectTest);
			CHECK(sprite.GetTexCoordsRect() == texcoordsRectTest);
			sprite.SetTexCoords(texCoordsRect);

			glm::mat4 transform2(1.f);
			transform2 = glm::translate(transform2, pos2);
			transform2 = glm::scale(transform2, scale2);
			sprite.SetTransform(transform2);
			CHECK(sprite.GetTransform() == transform2);

			//Actual sprite draw
			pig::Renderer2D::BeginScene(cameraController);
			pig::Renderer2D::DrawSprite(sprite);
			pig::Renderer2D::EndScene();

			REQUIRE(pig::TestingHelper::GetInstance().m_VertexBufferSetVertices.size() == 1);
			const pig::TestingHelper::VertexBufferSetVerticesData& interfaceData = pig::TestingHelper::GetInstance().m_VertexBufferSetVertices[0];
			CHECK(interfaceData.m_Count == 4);
			CHECK(interfaceData.m_CountOffset == 0);

			const int strideOffset = 10;

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.x + strideOffset * 0] == offsetNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.y + strideOffset * 0] == offsetNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.x + strideOffset * 1] == offsetNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.y + strideOffset * 1] == offsetNormalized.y + sizeNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.x + strideOffset * 2] == offsetNormalized.x + sizeNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.y + strideOffset * 2] == offsetNormalized.y + sizeNormalized.y);

			CHECK(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.x + strideOffset * 3] == offsetNormalized.x + sizeNormalized.x);
			CHECK(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.y + strideOffset * 3] == offsetNormalized.y);
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
			pig::Renderer2D::DrawString(stringTransform, testText, testFont, glm::vec4(col, 1.0f), textConfig.x, textConfig.y);
			pig::Renderer2D::EndScene();
			
			REQUIRE(pig::TestingHelper::GetInstance().m_VertexBufferSetVertices.size() == 1);
			const pig::TestingHelper::VertexBufferSetVerticesData& interfaceData = pig::TestingHelper::GetInstance().m_VertexBufferSetVertices[0];
			CHECK(interfaceData.m_Count == 44);
			CHECK(interfaceData.m_CountOffset == 0);

			const int strideOffset = 10;

			//Check texcoords char "S"
			glm::vec4 texcoordsRect(0.13525f, 0.3501f, 0.15479f, 0.31885f);
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.x + strideOffset * 0], texcoordsRect.x));
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.y + strideOffset * 0], texcoordsRect.y));

			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.x + strideOffset * 1], texcoordsRect.x));
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.y + strideOffset * 1], texcoordsRect.w));

			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.x + strideOffset * 2], texcoordsRect.z));
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.y + strideOffset * 2], texcoordsRect.w));

			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.x + strideOffset * 3], texcoordsRect.z));
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[texcoordsIndex.y + strideOffset * 3], texcoordsRect.y));

			//Check char pos "a"
			const glm::ivec3 posIndex(0, 1, 2);
			const int characterOffset_a(strideOffset * 6 * 0); //strideOffset * numVertices * characterIndex
			glm::vec4 posRect_a(1.07745f, 2.89154f, 2.54607f, 5.82879f);
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[posIndex.x + strideOffset * 0 + characterOffset_a], posRect_a.x));
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[posIndex.y + strideOffset * 0 + characterOffset_a], posRect_a.y));

			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[posIndex.x + strideOffset * 1 + characterOffset_a], posRect_a.x));
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[posIndex.y + strideOffset * 1 + characterOffset_a], posRect_a.w));

			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[posIndex.x + strideOffset * 2 + characterOffset_a], posRect_a.z));
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[posIndex.y + strideOffset * 2 + characterOffset_a], posRect_a.w));

			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[posIndex.x + strideOffset * 3 + characterOffset_a], posRect_a.z));
			CHECK(FLOAT_EQ(pig::TestingHelper::GetInstance().m_Vertices[posIndex.y + strideOffset * 3 + characterOffset_a], posRect_a.y));
		}
	}
	TEST_CASE("Core.Renderer2D::BatchRendering")
	{
		pig::Application& app = pig::CreateApplication();

		pig::OrthographicCameraController cameraController(1280.0f / 720.0f);
		const pig::Renderer2D::Data& data = pig::Renderer2D::GetData();

		const glm::vec3 pos1(4.f, 5.f, 6.f);
		const glm::vec3 col1(7.f, 8.f, 9.f);
		const glm::vec3 scale1(1.f, 2.f, 3.f);

		const glm::vec3 pos2(1.f, 2.f, 1.f);
		const glm::vec3 col2(4.f, 2.f, 1.f);
		const glm::vec3 scale2(2.f, 1.f, 2.f);

		const glm::vec3 pos3(4.f, 6.f, 1.f);
		const glm::vec3 col3(2.f, 3.f, 4.f);
		const glm::vec3 scale3(8.f, 1.f, 2.f);

		const glm::vec3 pos4(1.f, 2.f, 3.f);
		const glm::vec3 col4(11.f, 22.f, 33.f);
		const glm::vec3 scale4(4.f, 5.f, 6.f);
		const glm::vec2 textConfig(0.5f, 1.5f);

		const glm::vec3 origin1(0.5f, 0.5f, 0.f);
		const glm::vec3 origin2(1.5f, -0.5f, 1.f);

		glm::mat4 transform1(1.f);
		transform1 = glm::translate(transform1, pos1);
		transform1 = glm::scale(transform1, scale1);

		glm::mat4 transform2(1.f);
		transform2 = glm::translate(transform2, pos2);
		transform2 = glm::scale(transform2, scale2);

		glm::mat4 transform3(1.f);
		transform3 = glm::translate(transform3, pos3);
		transform3 = glm::scale(transform3, scale3);

		glm::mat4 transform4(1.f);
		transform4 = glm::translate(transform4, pos4);
		transform4 = glm::scale(transform4, scale4);

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

		pig::Renderer2D::DrawQuad(transform1, col1, origin1);
		pig::Renderer2D::DrawQuad(transform1, "SampleTexture1", origin2);
		pig::Renderer2D::DrawQuad(transform2, "SampleTexture1", origin1);
		pig::Renderer2D::DrawQuad(transform2, col2, origin2);
		pig::Renderer2D::DrawQuad(transform1, "SampleTexture2", origin1);
		pig::Renderer2D::DrawQuad(transform3, "SampleTexture2", origin2);
		pig::Renderer2D::DrawQuad(transform3, "SampleTexture1", origin1);
		pig::Renderer2D::DrawQuad(transform1, "SampleTexture3", origin1);
		pig::Renderer2D::DrawQuad(transform4, "SampleTexture3", origin2);
		pig::Renderer2D::DrawQuad(transform4, "SampleTexture2", origin1);
		pig::Renderer2D::DrawQuad(transform3, col3, origin2);

		const std::string testText("Sample\nText\n1");
		glm::mat4 stringTransform = glm::mat4(1.0f); // Identity matrix
		stringTransform = glm::translate(stringTransform, pos4); // Apply translation
		stringTransform = glm::scale(stringTransform, scale4); // Apply scaling
		pig::Renderer2D::DrawString(stringTransform, testText, testFont, glm::vec4(col4, 1.0f), textConfig.x, textConfig.y);

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

			CHECK(texBatch1.m_VertexBuffer[0] == 4.f);
			CHECK(texBatch1.m_IndexBuffer[0] == 0.f);
			CHECK(texBatch2.m_VertexBuffer[3] == 1.f);
			CHECK(texBatch2.m_IndexBuffer[5] == 3.f);
			CHECK(texBatch3.m_VertexBuffer[4] == 1.f);
			CHECK(texBatch3.m_IndexBuffer[6] == 4.f);
			CHECK(texBatch4.m_VertexBuffer[7] == 0.f);
			CHECK(texBatch4.m_IndexBuffer[11] == 7.f);
		}
		SECTION("Going over max count")
		{
			for (unsigned int i = 3; i < pig::Renderer2D::BATCH_MAX_COUNT; ++i)
			{
				pig::Renderer2D::DrawQuad(transform1, col1, origin1);
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

			pig::Renderer2D::DrawQuad(transform3, "SampleTexture1", origin1);
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

			pig::Renderer2D::DrawQuad(transform1, col1, origin2);

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

