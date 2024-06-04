#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>

#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>

#include <Pigeon/Renderer/Renderer2D.h>

#include <Pigeon/UI/UIComponents.h>
#include <Pigeon/UI/LayoutControlSystem.h>
#include <Pigeon/UI/UIRenderSystem.h>

#include <Platform/Testing/TestingHelper.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	class MockUIRenderSystemHelper : public pig::ui::IUIRenderSystemHelper
	{
	public:
		virtual void RendererBeginScene(const pig::OrthographicCamera& camera) override
		{
			m_SceneBegan = true;
		}
		virtual void RendererEndScene() override
		{
			m_SceneEnd = true;
		}
		virtual void RendererDrawQuad(const glm::mat4& transform, const pig::UUID& textureID, const glm::vec3& origin) override
		{
			m_TransformRender = transform;
			m_TextureID = textureID;
			m_Origin = origin;
		}
		virtual void RendererDrawString(const glm::mat4& transform, const std::string& string, pig::S_Ptr<pig::Font> /*font*/, const glm::vec4& color, float kerning, float linespacing) override
		{
			m_TransformRender = transform;
			m_String = string;
			m_Kerning = kerning;
			m_Spacing = linespacing;
			m_Color = color;
		}
		glm::vec2 GetStringBounds(const std::string& string, float kerning, float linespace, pig::S_Ptr<pig::Font> font)
		{
			return m_BoundsToReturn;
		}

		unsigned int GetStringNumLines(const std::string& string, pig::S_Ptr<pig::Font> font)
		{
			return m_LinesToReturn;
		}

		virtual pig::S_Ptr<pig::Font> CreateUIFont() override
		{
			m_FontCreated = true;
			return nullptr;
		}

		void Reset()
		{
			m_SceneBegan = false;
			m_SceneEnd = false;
			m_TextDrawn = false;
			m_ImageDrawn = false;
		}
		bool m_FontCreated = false;
		bool m_SceneBegan = false;
		bool m_SceneEnd = false;
		bool m_TextDrawn = false;
		bool m_ImageDrawn = false;
		glm::mat4 m_TransformRender{};
		std::string m_String{};
		pig::UUID m_TextureID;
		glm::vec3 m_Origin{};
		glm::vec4 m_Color{};
		float m_Spacing{};
		float m_Kerning{};

		glm::vec2 m_BoundsToReturn{};
		unsigned int m_LinesToReturn = 0;
	};

	void TestImage(pig::S_Ptr<MockUIRenderSystemHelper> helper, const pig::UUID& texture, const glm::vec2& position, const glm::vec2& size, float z)
	{
		CHECK(helper->m_SceneBegan);
		CHECK(helper->m_SceneEnd);
		CHECK(helper->m_TextureID == texture);
		glm::mat4 transform(1.f);
		transform = glm::translate(transform, glm::vec3(position, z));
		transform = glm::scale(transform, glm::vec3(size, 1.f));

		CHECK(helper->m_TransformRender == transform);
	}

	void TestText(pig::S_Ptr<MockUIRenderSystemHelper> helper, const glm::vec2& position, const glm::vec2& size, const std::string& string, const glm::vec4& color, float kerning, float linespacing, float z)
	{
		glm::mat4 transform(1.f);
		transform = glm::translate(transform, glm::vec3(position, z));
		transform = glm::scale(transform, glm::vec3(size, 1.f));

		CHECK(helper->m_TransformRender == transform);
		CHECK(helper->m_Color == color);
		CHECK(helper->m_Spacing == linespacing);
		CHECK(helper->m_Kerning == kerning);
		CHECK(helper->m_String == string);
	}
}

//ARNAU TODO Rename this file to a better name
namespace CatchTestsetFail
{
	TEST_CASE("UI.UIRenderSystem::UIRenderSystem")
	{
		pig::World& world = pig::World::Create();

		entt::entity uiElementEntity = pig::World::GetRegistry().create();

		pig::S_Ptr<MockUIRenderSystemHelper> helper = std::make_shared<MockUIRenderSystemHelper>();

		world.RegisterSystem(std::move(std::make_unique<pig::ui::UIRenderSystem>(helper)));

		pig::ui::BaseComponent& baseComponent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(uiElementEntity);
		pig::UUID sampleTextureID = pig::Renderer2D::AddTexture("Assets/Test/SampleTexture.png", pig::EMappedTextureType::eQuad);
		baseComponent.m_Size = { 100.f, 200.f };

		auto viewRenderConfig = pig::World::GetRegistry().view<const pig::ui::RendererConfig>();
		CHECK(viewRenderConfig.size() == 0);
		pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());

		viewRenderConfig = pig::World::GetRegistry().view<const pig::ui::RendererConfig>();
		REQUIRE(viewRenderConfig.size() == 1);

		const pig::ui::RendererConfig& renderComponent = viewRenderConfig.get<pig::ui::RendererConfig>(viewRenderConfig.front());
		renderComponent.m_Camera.GetData();
		CHECK(helper->m_FontCreated);
		CHECK(FLOAT_EQ(renderComponent.m_Width, 1920.f));
		CHECK(FLOAT_EQ(renderComponent.m_Height, 1080.f));

		SECTION("Render UI image")
		{
			pig::ui::ImageComponent& imageComponent = pig::World::GetRegistry().emplace<pig::ui::ImageComponent>(uiElementEntity);
			imageComponent.m_TextureHandle = sampleTextureID;
			
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());

			TestImage(helper, imageComponent.m_TextureHandle, baseComponent.m_Spacing, baseComponent.m_Size, 0.f);

			baseComponent.m_Size = { 300.f, 100.f };
			baseComponent.m_Spacing = { 10.f, 20.f};

			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());

			TestImage(helper, imageComponent.m_TextureHandle, baseComponent.m_Spacing, baseComponent.m_Size, 0.f);
			helper->Reset();
			pig::World::GetRegistry().destroy(uiElementEntity);
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			CHECK(!helper->m_ImageDrawn);
		}
		SECTION("Render UI text")
		{
			pig::ui::TextComponent& textComponent = pig::World::GetRegistry().emplace<pig::ui::TextComponent>(uiElementEntity);
			textComponent.m_Text = "This is a\nSample Text 1";
			textComponent.m_Color = {1.f, 0.f, 0.f, 1.f};
			textComponent.m_Kerning = 10.f;
			textComponent.m_Spacing = 2.f;
			helper->m_LinesToReturn = 2;
			baseComponent.m_Size = { 300.f, 200.f };

			glm::vec2 sizeToCheck{};
			SECTION("Width limited")
			{
				helper->m_BoundsToReturn = { 100.f, 20.f };
				sizeToCheck.x = baseComponent.m_Size.x / helper->m_BoundsToReturn.x;
				sizeToCheck.y = baseComponent.m_Size.x / helper->m_BoundsToReturn.x;
			}
			SECTION("Height limited")
			{
				helper->m_BoundsToReturn = { 20.f, 100.f };
				sizeToCheck.x = (baseComponent.m_Size.y / helper->m_BoundsToReturn.y) / helper->m_LinesToReturn;
				sizeToCheck.y = (baseComponent.m_Size.y / helper->m_BoundsToReturn.y) / helper->m_LinesToReturn;
			}

			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestText(helper, baseComponent.m_Spacing, sizeToCheck, textComponent.m_Text, textComponent.m_Color, textComponent.m_Kerning, textComponent.m_Spacing, 0.f);
		}
		SECTION("Test alignment")
		{
			pig::ui::ImageComponent& imageComponent = pig::World::GetRegistry().emplace<pig::ui::ImageComponent>(uiElementEntity);
			imageComponent.m_TextureHandle = sampleTextureID;

			baseComponent.m_Size = { 300.f, 100.f };
			baseComponent.m_Spacing = { 50.f, 60.f };
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, baseComponent.m_Spacing, baseComponent.m_Size, 0.f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eLeft;
			baseComponent.m_VAlign = pig::ui::EVAlignType::eTop;
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, baseComponent.m_Spacing, baseComponent.m_Size, 0.f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eRight;
			glm::vec2 posFinal = glm::vec2(renderComponent.m_Width - (baseComponent.m_Size.x + baseComponent.m_Spacing.x), baseComponent.m_Spacing.y);
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);

			baseComponent.m_VAlign = pig::ui::EVAlignType::eBottom;
			posFinal = glm::vec2(renderComponent.m_Width - (baseComponent.m_Size.x + baseComponent.m_Spacing.x), renderComponent.m_Height - (baseComponent.m_Size.y + baseComponent.m_Spacing.y));
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eCenter;
			posFinal = glm::vec2((renderComponent.m_Width/2.f - baseComponent.m_Size.x/2.f) + baseComponent.m_Spacing.x, renderComponent.m_Height - (baseComponent.m_Size.y + baseComponent.m_Spacing.y));
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);

			baseComponent.m_VAlign = pig::ui::EVAlignType::eCenter;
			posFinal = glm::vec2((renderComponent.m_Width / 2.f - baseComponent.m_Size.x / 2.f) + baseComponent.m_Spacing.x, (renderComponent.m_Height/2.f - baseComponent.m_Size.y/2.f) + baseComponent.m_Spacing.y);
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eLeft;
			posFinal = glm::vec2(baseComponent.m_Spacing.x, (renderComponent.m_Height / 2.f - baseComponent.m_Size.y / 2.f) + baseComponent.m_Spacing.y);
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);
		}
		SECTION("Render multilevel UI")
		{
			pig::ui::ImageComponent& imageComponent = pig::World::GetRegistry().emplace<pig::ui::ImageComponent>(uiElementEntity);
			imageComponent.m_TextureHandle = sampleTextureID;

			entt::entity uiElementEntityParent = pig::World::GetRegistry().create();

			baseComponent.m_Parent = uiElementEntityParent;
			baseComponent.m_Size = { 300.f, 100.f };
			baseComponent.m_Spacing = { 50.f, 60.f };

			pig::ui::BaseComponent& baseComponentParent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(uiElementEntityParent);
			entt::entity uiElementEntityParentParent = pig::World::GetRegistry().create();
			baseComponentParent.m_Size = { 600.f, 800.f };
			baseComponentParent.m_Spacing = { 10.f, 20.f };
			baseComponentParent.m_Parent = uiElementEntityParentParent;

			pig::ui::BaseComponent& baseComponentParentParent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(uiElementEntityParentParent);
			baseComponentParentParent.m_Size = { 1000.f, 900.f };
			baseComponentParentParent.m_Spacing = { 100.f, 10.f };

			glm::vec2 posFinal{};
			posFinal.x = 50 + 10 + 100; //160
			posFinal.y = 10 + 20 + 60; // 90

			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponentParent.m_HAlign = pig::ui::EHAlignType::eRight;
			posFinal.x = 50 + 100 + 1000 - (10 + 600); //540
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponentParentParent.m_HAlign = pig::ui::EHAlignType::eRight;
			posFinal.x = 1920 - (1000 + 100) + 1000 - (10 + 600) + 50; // 820 + 390 + 50 = 1260
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);
			
			baseComponentParent.m_HAlign = pig::ui::EHAlignType::eCenter;
			posFinal.x = 1920 - (1000 + 100) + (500 - 300) + 10 + 50; // 820 + 210 + 50 = 1080
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);
			
			baseComponentParent.m_VAlign = pig::ui::EVAlignType::eCenter;
			posFinal.y = 10 + (450 - 400) + 20 + 60; //10 + 70 + 60 = 140
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponentParentParent.m_VAlign = pig::ui::EVAlignType::eBottom;
			posFinal.y = 1080 - (900 + 10) + (450 - 400) + 20 + 60; //170 + 70 + 60 = 140
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eRight;
			posFinal.x = 1920 - (1000 + 100) + (500 - 300) + 10 + 600 - (300 + 50); // 820 + 210 + 250 = 1280
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponent.m_VAlign = pig::ui::EVAlignType::eCenter;
			posFinal.y = 1080 - (900 + 10) + (450 - 400) + 20 + (400 - 50) + 60; //170 + 70 + 410 = 650
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			TestImage(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);
		}
	}
	TEST_CASE("UI.LayoutControlSystem::LayoutControlSystem")
	{
		//pig::World& world = pig::World::Create();

		//world.RegisterSystem(std::move(std::make_unique<pig::ui::UIRenderSystem>()));

		//entt::entity layoutEntity = pig::World::GetRegistry().create();

		//ARNAU TODO: automatize one frame components?
		//pig::ui::LoadableLayoutOneFrameComponent& layoutComponent = pig::World::GetRegistry().emplace<pig::ui::LoadableLayoutOneFrameComponent>(layoutEntity);

		/*SECTION("TestUI1")
		{
			layoutComponent.m_LayoutFilePath = "Assets/Test/TestUISmall1.json";
			pig::World::Get().Update(pig::Timestep(0).AsMilliseconds());
			pig::World::GetRegistry().destroy(layoutEntity);

			auto view = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::TextComponent>();

			u_int count = 0;
			for (auto ent : view)
			{
				const pig::ui::BaseComponent& baseComponent = view.get<pig::ui::BaseComponent>(ent);
				CHECK(FLOAT_EQ(baseComponent.m_Pos.x, 100.f));
				CHECK(FLOAT_EQ(baseComponent.m_Pos.y, 200.f));
				CHECK(FLOAT_EQ(baseComponent.m_Pos.z, 0.f));

				CHECK(FLOAT_EQ(baseComponent.m_Size.x, 400.f));
				CHECK(FLOAT_EQ(baseComponent.m_Size.y, 50));

				CHECK(FLOAT_EQ(baseComponent.m_Spacing.x, 5));
				CHECK(FLOAT_EQ(baseComponent.m_Spacing.y, 10));

				CHECK(baseComponent.m_HAlign == pig::ui::EHAlignType::eLeft);
				CHECK(baseComponent.m_VAlign == pig::ui::EVAlignType::eNone);

				const pig::ui::TextComponent& textComponent = view.get<pig::ui::TextComponent>(ent);
				CHECK(textComponent.m_Text == std::string("this is a sample text"));
				CHECK(textComponent.m_FontSize == 32);
				count++;
			}
			CHECK(count == 1);
		}*/
		/*
		pig::ui::ElementComponent& component = pig::World::GetRegistry().emplace<pig::ui::ElementComponent>(testEntity);
		component.m_Pos = glm::vec3(1.f, 2.f, 3.f);

		app.TestUpdate(pig::Timestep(0));

		auto view = pig::World::GetRegistry().view<pig::ui::ElementComponent>();
		REQUIRE(view.size() == 1);
		for (auto ent : view)
		{
			pig::ui::ElementComponent& component = view.get<pig::ui::ElementComponent>(ent);
			CHECK(component.m_Pos.x == 0.f);
			CHECK(component.m_Pos.y == 0.f);
			CHECK(component.m_Pos.z == 0.f);
		}*/
		// JSON string
		/*std::string jsonString = R"({
			"name": "John Doe",
			"age": 30,
			"is_student": false,
			"address": {
				"street": "123 Main St",
				"city": "Anytown"
			},
			"phone_numbers": ["123-456-7890", "987-654-3210"]
		})";

		// Parse the JSON string
		json jsonObject = json::parse(jsonString);*/
	}
}