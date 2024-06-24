#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>

#include "Utils/TestApp.h"

#include <Pigeon/Core/InputComponents.h>

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>

#include <Pigeon/Renderer/Renderer2D.h>

#include <Pigeon/UI/UIComponents.h>
#include <Pigeon/UI/UIControlSystem.h>
#include <Pigeon/UI/UIEventSystem.h>
#include <Pigeon/UI/UIRenderSystem.h>

#include <Platform/Testing/TestingHelper.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	class MockUIControlSystemHelper : public pig::ui::IUIControlSystemHelper
	{
	public:
		virtual pig::UUID CreateUIImageFromPath(const std::string& path) override
		{
			return pig::UUID::Generate();
		}
	};

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
			m_ImageDrawn = true;
		}
		virtual void RendererDrawString(const glm::mat4& transform, const std::string& string, pig::S_Ptr<pig::Font> /*font*/, const glm::vec4& color, float kerning, float linespacing) override
		{
			m_TransformRender = transform;
			m_String = string;
			m_Kerning = kerning;
			m_Spacing = linespacing;
			m_Color = color;
			m_ImageDrawn = true;
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

	void TestUIRender(pig::S_Ptr<MockUIRenderSystemHelper> helper, const pig::UUID& texture, const glm::vec2& position, const glm::vec2& size, float z)
	{
		CHECK(helper->m_SceneBegan);
		CHECK(helper->m_SceneEnd);
		CHECK(helper->m_TextureID == texture);
		CHECK(helper->m_ImageDrawn);
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
		CHECK(helper->m_ImageDrawn);
	}
}

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
		pig::World::Get().Update(pig::Timestep(0));

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
			
			pig::World::Get().Update(pig::Timestep(0));

			TestUIRender(helper, imageComponent.m_TextureHandle, baseComponent.m_Spacing, baseComponent.m_Size, 0.f);

			baseComponent.m_Size = { 300.f, 100.f };
			baseComponent.m_Spacing = { 10.f, 20.f};

			pig::World::Get().Update(pig::Timestep(0));

			TestUIRender(helper, imageComponent.m_TextureHandle, baseComponent.m_Spacing, baseComponent.m_Size, 0.f);
			helper->Reset();
			pig::World::GetRegistry().destroy(uiElementEntity);
			pig::World::Get().Update(pig::Timestep(0));
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

			pig::World::Get().Update(pig::Timestep(0));
			TestText(helper, baseComponent.m_Spacing, sizeToCheck, textComponent.m_Text, textComponent.m_Color, textComponent.m_Kerning, textComponent.m_Spacing, 0.f);
		}

		SECTION("Render disabled element")
		{
			pig::ui::ImageComponent& imageComponent = pig::World::GetRegistry().emplace<pig::ui::ImageComponent>(uiElementEntity);
			imageComponent.m_TextureHandle = sampleTextureID;
			baseComponent.m_Parent = pig::World::GetRegistry().create();

			pig::ui::BaseComponent& baseComponentParent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(baseComponent.m_Parent);
			baseComponentParent.m_Parent = pig::World::GetRegistry().create();
			pig::ui::BaseComponent& baseComponentParentParent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(baseComponentParent.m_Parent);

			SECTION("base")
			{
				baseComponent.m_Enabled = false;
				pig::World::Get().Update(pig::Timestep(0));
				CHECK(!helper->m_ImageDrawn);

				baseComponent.m_Enabled = true;
				pig::World::Get().Update(pig::Timestep(0));
				CHECK(helper->m_ImageDrawn);
			}
			SECTION("parent")
			{
				baseComponentParent.m_Enabled = false;
				pig::World::Get().Update(pig::Timestep(0));
				CHECK(!helper->m_ImageDrawn);

				baseComponentParent.m_Enabled = true;
				pig::World::Get().Update(pig::Timestep(0));
				CHECK(helper->m_ImageDrawn);
			}
			SECTION("root parent")
			{
				baseComponentParentParent.m_Enabled = false;
				pig::World::Get().Update(pig::Timestep(0));
				CHECK(!helper->m_ImageDrawn);

				baseComponentParentParent.m_Enabled = true;
				pig::World::Get().Update(pig::Timestep(0));
				CHECK(helper->m_ImageDrawn);
			}
		}

		SECTION("Test alignment")
		{
			pig::ui::ImageComponent& imageComponent = pig::World::GetRegistry().emplace<pig::ui::ImageComponent>(uiElementEntity);
			imageComponent.m_TextureHandle = sampleTextureID;

			baseComponent.m_Size = { 300.f, 100.f };
			baseComponent.m_Spacing = { 50.f, 60.f };
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, baseComponent.m_Spacing, baseComponent.m_Size, 0.f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eLeft;
			baseComponent.m_VAlign = pig::ui::EVAlignType::eTop;
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, baseComponent.m_Spacing, baseComponent.m_Size, 0.f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eRight;
			glm::vec2 posFinal = glm::vec2(renderComponent.m_Width - (baseComponent.m_Size.x + baseComponent.m_Spacing.x), baseComponent.m_Spacing.y);
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);

			baseComponent.m_VAlign = pig::ui::EVAlignType::eBottom;
			posFinal = glm::vec2(renderComponent.m_Width - (baseComponent.m_Size.x + baseComponent.m_Spacing.x), renderComponent.m_Height - (baseComponent.m_Size.y + baseComponent.m_Spacing.y));
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eCenter;
			posFinal = glm::vec2((renderComponent.m_Width/2.f - baseComponent.m_Size.x/2.f) + baseComponent.m_Spacing.x, renderComponent.m_Height - (baseComponent.m_Size.y + baseComponent.m_Spacing.y));
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);

			baseComponent.m_VAlign = pig::ui::EVAlignType::eCenter;
			posFinal = glm::vec2((renderComponent.m_Width / 2.f - baseComponent.m_Size.x / 2.f) + baseComponent.m_Spacing.x, (renderComponent.m_Height/2.f - baseComponent.m_Size.y/2.f) + baseComponent.m_Spacing.y);
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eLeft;
			posFinal = glm::vec2(baseComponent.m_Spacing.x, (renderComponent.m_Height / 2.f - baseComponent.m_Size.y / 2.f) + baseComponent.m_Spacing.y);
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, 0.f);
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

			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponentParent.m_HAlign = pig::ui::EHAlignType::eRight;
			posFinal.x = 50 + 100 + 1000 - (10 + 600); //540
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponentParentParent.m_HAlign = pig::ui::EHAlignType::eRight;
			posFinal.x = 1920 - (1000 + 100) + 1000 - (10 + 600) + 50; // 820 + 390 + 50 = 1260
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);
			
			baseComponentParent.m_HAlign = pig::ui::EHAlignType::eCenter;
			posFinal.x = 1920 - (1000 + 100) + (500 - 300) + 10 + 50; // 820 + 210 + 50 = 1080
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);
			
			baseComponentParent.m_VAlign = pig::ui::EVAlignType::eCenter;
			posFinal.y = 10 + (450 - 400) + 20 + 60; //10 + 70 + 60 = 140
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponentParentParent.m_VAlign = pig::ui::EVAlignType::eBottom;
			posFinal.y = 1080 - (900 + 10) + (450 - 400) + 20 + 60; //170 + 70 + 60 = 140
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponent.m_HAlign = pig::ui::EHAlignType::eRight;
			posFinal.x = 1920 - (1000 + 100) + (500 - 300) + 10 + 600 - (300 + 50); // 820 + 210 + 250 = 1280
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);

			baseComponent.m_VAlign = pig::ui::EVAlignType::eCenter;
			posFinal.y = 1080 - (900 + 10) + (450 - 400) + 20 + (400 - 50) + 60; //170 + 70 + 410 = 650
			pig::World::Get().Update(pig::Timestep(0));
			TestUIRender(helper, imageComponent.m_TextureHandle, posFinal, baseComponent.m_Size, -0.2f);
		}
	}
	TEST_CASE("UI.UIEventSystem::UIEventSystem")
	{
		pig::World& world = pig::World::Create();

		entt::entity inputEventsEntity = pig::World::GetRegistry().create();

		world.RegisterSystem(std::move(std::make_unique<pig::ui::UIEventSystem>()));

		pig::InputStateSingletonComponent& inputComponent = pig::World::GetRegistry().emplace<pig::InputStateSingletonComponent>(inputEventsEntity);
		
		pig::ui::BaseComponent& baseComponent1 = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(pig::World::GetRegistry().create());

		pig::ui::RendererConfig& renderComponent = pig::World::GetRegistry().emplace<pig::ui::RendererConfig>(pig::World::GetRegistry().create());
		renderComponent.m_Height = 800;
		renderComponent.m_Width = 900;

		baseComponent1.m_Size = { 200.f, 100.f };
		baseComponent1.m_Spacing = { 10.f, 20.f };

		baseComponent1.m_UUID = pig::UUID::Generate();

		pig::World::Get().Update(pig::Timestep(0));
		{
			auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
			auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
			auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();

			CHECK(viewClick.size() == 0);
			CHECK(viewHover.size() == 0);
			CHECK(viewRelease.size() == 0);
		}
		
		SECTION("Check single UI corners hover")
		{
			inputComponent.m_MousePos = { 9.f, 19.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 0);
			}
			inputComponent.m_MousePos = { 11.f, 21.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();

				REQUIRE(viewHover.size() == 1);

				const pig::ui::UIOnHoverOneFrameComponent& hoverComponent = viewHover.get<pig::ui::UIOnHoverOneFrameComponent>(viewHover.front());
				CHECK(hoverComponent.m_ElementID == baseComponent1.m_UUID);
			}
			inputComponent.m_MousePos = { 209.f, 119.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();

				REQUIRE(viewHover.size() == 1);

				const pig::ui::UIOnHoverOneFrameComponent& hoverComponent = viewHover.get<pig::ui::UIOnHoverOneFrameComponent>(viewHover.front());
				CHECK(hoverComponent.m_ElementID == baseComponent1.m_UUID);
			}
			inputComponent.m_MousePos = { 211.f, 121.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 0);
			}
			inputComponent.m_MousePos = { 11.f, 119.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				REQUIRE(viewHover.size() == 1);

				const pig::ui::UIOnHoverOneFrameComponent& hoverComponent = viewHover.get<pig::ui::UIOnHoverOneFrameComponent>(viewHover.front());
				CHECK(hoverComponent.m_ElementID == baseComponent1.m_UUID);
			}
			inputComponent.m_MousePos = { 9.f, 119.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 0);
			}
			inputComponent.m_MousePos = { 211.f, 21.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 0);
			}
			inputComponent.m_MousePos = { 209.f, 21.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				REQUIRE(viewHover.size() == 1);

				const pig::ui::UIOnHoverOneFrameComponent& hoverComponent = viewHover.get<pig::ui::UIOnHoverOneFrameComponent>(viewHover.front());
				CHECK(hoverComponent.m_ElementID == baseComponent1.m_UUID);
			}
			baseComponent1.m_Spacing = { 11.f, 20.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				REQUIRE(viewHover.size() == 1);

				const pig::ui::UIOnHoverOneFrameComponent& hoverComponent = viewHover.get<pig::ui::UIOnHoverOneFrameComponent>(viewHover.front());
				CHECK(hoverComponent.m_ElementID == baseComponent1.m_UUID);
			}
			baseComponent1.m_Spacing = { 8.f, 20.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 0);
			}
		}
		SECTION("Check single UI click")
		{
			inputComponent.m_MousePos = { 5.f, 5.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
				auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();

				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 0);
				CHECK(viewClick.size() == 0);
				CHECK(viewRelease.size() == 0);
			}
			inputComponent.m_KeysPressed[pig::PG_MOUSE_BUTTON_LEFT] = 1;
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
				auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();

				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 0);
				CHECK(viewClick.size() == 0);
				CHECK(viewRelease.size() == 0);
			}
			inputComponent.m_MousePos = { 30.f, 30.f };
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
				auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();

				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 1);
				CHECK(viewRelease.size() == 0);

				REQUIRE(viewClick.size() == 1);

				const pig::ui::UIOnClickOneFrameComponent& clickComponent = viewClick.get<pig::ui::UIOnClickOneFrameComponent>(viewClick.front());
				CHECK(clickComponent.m_ElementID == baseComponent1.m_UUID);
			}
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
				auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();

				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 1);
				CHECK(viewRelease.size() == 0);
				CHECK(viewClick.size() == 1);
			}
			inputComponent.m_KeysReleased[pig::PG_MOUSE_BUTTON_LEFT] = 1;
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
				auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();

				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 1);
				CHECK(viewRelease.size() == 0);
				CHECK(viewClick.size() == 1);
			}
			inputComponent.m_KeysPressed.erase(pig::PG_MOUSE_BUTTON_LEFT);
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
				auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();

				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 1);
				CHECK(viewClick.size() == 0);

				REQUIRE(viewRelease.size() == 1);

				const pig::ui::UIOnReleaseOneFrameComponent& releaseComponent = viewRelease.get<pig::ui::UIOnReleaseOneFrameComponent>(viewRelease.front());
				CHECK(releaseComponent.m_ElementID == baseComponent1.m_UUID);
			}
			inputComponent.m_KeysReleased.erase(pig::PG_MOUSE_BUTTON_LEFT);
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
				auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();

				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 1);
				CHECK(viewRelease.size() == 0);
				CHECK(viewClick.size() == 0);
			}
		}

		SECTION("Check events on disabled element")
		{
			baseComponent1.m_Parent = pig::World::GetRegistry().create();

			pig::ui::BaseComponent& baseComponentParent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(baseComponent1.m_Parent);
			baseComponentParent.m_Parent = pig::World::GetRegistry().create();
			pig::ui::BaseComponent& baseComponentParentParent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(baseComponentParent.m_Parent);

			baseComponentParent.m_Enabled = false;

			inputComponent.m_MousePos = { 30.f, 30.f };
			inputComponent.m_KeysPressed[pig::PG_MOUSE_BUTTON_LEFT] = 1;
			pig::World::Get().Update(pig::Timestep(0));
			{
				auto viewClick = pig::World::GetRegistry().view<pig::ui::UIOnClickOneFrameComponent>();
				auto viewRelease = pig::World::GetRegistry().view<pig::ui::UIOnReleaseOneFrameComponent>();
				auto viewHover = pig::World::GetRegistry().view<pig::ui::UIOnHoverOneFrameComponent>();
				CHECK(viewHover.size() == 0);
				CHECK(viewRelease.size() == 0);
				CHECK(viewClick.size() == 0);
			}
		}
	}
	TEST_CASE("UI.UIControlSystem::UpdateEvents")
	{
		pig::World& world = pig::World::Create();


		pig::S_Ptr<MockUIControlSystemHelper> helper = std::make_shared<MockUIControlSystemHelper>();

		world.RegisterSystem(std::move(std::make_unique<pig::ui::UIControlSystem>(helper)));

		entt::entity layoutEntity = pig::World::GetRegistry().create();

		pig::ui::BaseComponent& baseComponent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(layoutEntity);
		pig::ui::RendererConfig& renderComponent = pig::World::GetRegistry().emplace<pig::ui::RendererConfig>(pig::World::GetRegistry().create());
		renderComponent.m_Height = 800;
		renderComponent.m_Width = 900;

		baseComponent.m_Size = { 200.f, 100.f };
		baseComponent.m_Spacing = { 10.f, 20.f };

		baseComponent.m_UUID = pig::UUID::Generate();
		baseComponent.m_Parent = entt::null;

		SECTION("Update transform")
		{
			pig::ui::UIUpdateTransformOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateTransformOneFrameComponent>(layoutEntity);
			updateComponent.m_HAlign = pig::ui::EHAlignType::eLeft;
			updateComponent.m_VAlign = pig::ui::EVAlignType::eBottom;
			updateComponent.m_Size = { 123.f, 456.f };
			updateComponent.m_Spacing = { 44.f, 55.f };
			pig::World::Get().Update(pig::Timestep(0));

			auto viewUpdated = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
			REQUIRE(viewUpdated.size() == 1);
			const pig::ui::BaseComponent& baseComponentUpdated = viewUpdated.get<const pig::ui::BaseComponent>(layoutEntity);

			CHECK(baseComponentUpdated.m_HAlign == pig::ui::EHAlignType::eLeft);
			CHECK(baseComponentUpdated.m_VAlign == pig::ui::EVAlignType::eBottom);
			CHECK(baseComponentUpdated.m_Size == glm::vec2(123.f, 456.f));
			CHECK(baseComponentUpdated.m_Spacing == glm::vec2(44.f, 55.f));
		}
		SECTION("Update parent")
		{
			pig::ui::UIUpdateParentOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateParentOneFrameComponent>(layoutEntity);
			entt::entity entityParent = pig::World::GetRegistry().create();
			updateComponent.m_Parent = entityParent;
			pig::World::Get().Update(pig::Timestep(0));

			auto viewUpdated = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
			REQUIRE(viewUpdated.size() == 1);
			const pig::ui::BaseComponent& baseComponentUpdated = viewUpdated.get<const pig::ui::BaseComponent>(layoutEntity);

			CHECK(baseComponentUpdated.m_Parent == entityParent);
		}
		SECTION("Update enabled")
		{
			pig::ui::UIUpdateEnableOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateEnableOneFrameComponent>(layoutEntity);
			updateComponent.m_Enabled = false;
			pig::World::Get().Update(pig::Timestep(0));

			auto viewUpdated = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
			REQUIRE(viewUpdated.size() == 1);
			const pig::ui::BaseComponent& baseComponentUpdated = viewUpdated.get<const pig::ui::BaseComponent>(layoutEntity);

			CHECK(baseComponentUpdated.m_Enabled == false);
		}
		SECTION("Update id")
		{
			pig::ui::UIUpdateUUIDOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateUUIDOneFrameComponent>(layoutEntity);
			pig::UUID updateID = pig::UUID::Generate();
			updateComponent.m_UUID = updateID;
			pig::World::Get().Update(pig::Timestep(0));

			auto viewUpdated = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
			REQUIRE(viewUpdated.size() == 1);
			const pig::ui::BaseComponent& baseComponentUpdated = viewUpdated.get<const pig::ui::BaseComponent>(layoutEntity);

			CHECK(baseComponentUpdated.m_UUID == updateID);
		}
		SECTION("Update image")
		{
			pig::ui::ImageComponent& imageComponent = pig::World::GetRegistry().emplace<pig::ui::ImageComponent>(layoutEntity);
			imageComponent.m_TextureHandle = pig::UUID::Generate();

			pig::ui::UIUpdateImageUUIDOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateImageUUIDOneFrameComponent>(layoutEntity);
			pig::UUID updateID = pig::UUID::Generate();
			updateComponent.m_UUID = updateID;
			updateComponent.m_PreviousImageToDestroy = pig::UUID::Generate();
			
			pig::World::Get().Update(pig::Timestep(0));

			auto viewUpdated = pig::World::GetRegistry().view<const pig::ui::ImageComponent>();
			REQUIRE(viewUpdated.size() == 1);
			const pig::ui::ImageComponent& imageComponentUpdated = viewUpdated.get<const pig::ui::ImageComponent>(layoutEntity);

			CHECK(imageComponentUpdated.m_TextureHandle == updateID);
		}
		SECTION("Update text")
		{
			pig::ui::TextComponent& textComponent = pig::World::GetRegistry().emplace<pig::ui::TextComponent>(layoutEntity);
			textComponent.m_Color = { 0.f, 0.f, 0.f, 0.f };
			textComponent.m_Kerning = 0.f;
			textComponent.m_Spacing = 0.f;
			textComponent.m_Text = {};

			pig::ui::UIUpdateTextOneFrameComponent& updateComponent = pig::World::GetRegistry().emplace<pig::ui::UIUpdateTextOneFrameComponent>(layoutEntity);
			updateComponent.m_Color = { 1.f, 2.f, 3.f, 4.f };
			updateComponent.m_Kerning = 5.f;
			updateComponent.m_Spacing = 6.f;
			updateComponent.m_Text = "random text";

			pig::World::Get().Update(pig::Timestep(0));

			auto viewUpdated = pig::World::GetRegistry().view<const pig::ui::TextComponent>();
			REQUIRE(viewUpdated.size() == 1);
			const pig::ui::TextComponent& textComponentUpdated = viewUpdated.get<const pig::ui::TextComponent>(layoutEntity);

			CHECK(textComponentUpdated.m_Color == glm::vec4(1.f, 2.f, 3.f, 4.f));
			CHECK(textComponentUpdated.m_Kerning == 5.f);
			CHECK(textComponentUpdated.m_Spacing == 6.f);
			CHECK(textComponentUpdated.m_Text == "random text");
		}
		SECTION("Destroy UI")
		{

			baseComponent.m_Parent = pig::World::GetRegistry().create();

			pig::ui::BaseComponent& baseComponentParent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(baseComponent.m_Parent);
			baseComponentParent.m_Parent = pig::World::GetRegistry().create();
			pig::ui::BaseComponent& baseComponentParentParent = pig::World::GetRegistry().emplace<pig::ui::BaseComponent>(baseComponentParent.m_Parent);

			entt::entity baseEnt = layoutEntity;
			entt::entity parentEnt = baseComponent.m_Parent;
			entt::entity rootEnt = baseComponentParent.m_Parent;

			pig::ui::UIDestroyOneFrameComponent& destroyComponent = pig::World::GetRegistry().emplace<pig::ui::UIDestroyOneFrameComponent>(parentEnt);
			pig::World::Get().Update(pig::Timestep(0));

			CHECK(!pig::World::GetRegistry().valid(baseEnt));
			CHECK(!pig::World::GetRegistry().valid(parentEnt));
			CHECK(pig::World::GetRegistry().valid(rootEnt));
		}
	}
	TEST_CASE("UI.UIControlSystem::LoadFromFileEvents")
	{
		pig::World& world = pig::World::Create();
		pig::S_Ptr<MockUIControlSystemHelper> helper = std::make_shared<MockUIControlSystemHelper>();

		world.RegisterSystem(std::move(std::make_unique<pig::ui::UIControlSystem>(helper)));

		SECTION("Load layout from file")
		{
			pig::ui::LoadLayoutOneFrameComponent& layoutJson = pig::World::GetRegistry().emplace<pig::ui::LoadLayoutOneFrameComponent>(pig::World::GetRegistry().create());
			
			SECTION("Load single element image layout")
			{
				layoutJson.m_LayoutFilePath = "Assets/Test/TestUISmall1.json";
				pig::World::Get().Update(pig::Timestep(0));

				int elementCount = 0;
				auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::ImageComponent>();
				for (auto ent : viewUI)
				{
					const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
					const pig::ui::ImageComponent& dataComponent = viewUI.get<pig::ui::ImageComponent>(ent);
					CHECK(baseComponent.m_HAlign == pig::ui::EHAlignType::eCenter);
					CHECK(baseComponent.m_VAlign == pig::ui::EVAlignType::eTop);
					CHECK(bool(baseComponent.m_Parent == entt::null));
					CHECK(baseComponent.m_Spacing == glm::vec2(123.4f, 56.7f));
					CHECK(baseComponent.m_Size == glm::vec2(4.4f, 5.7f));
					CHECK(baseComponent.m_UUID == pig::UUID("12345678-9abc-def0-1234-56789abcdef1"));
					CHECK(baseComponent.m_Enabled);

					CHECK(!dataComponent.m_TextureHandle.IsNull());
					elementCount++;
				}
				CHECK(elementCount == 1);
			}
			SECTION("Load single element text layout")
			{
				layoutJson.m_LayoutFilePath = "Assets/Test/TestUISmall2.json";
				pig::World::Get().Update(pig::Timestep(0));

				int elementCount = 0;
				auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::TextComponent>();
				for (auto ent : viewUI)
				{
					const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
					const pig::ui::TextComponent& dataComponent = viewUI.get<pig::ui::TextComponent>(ent);
					CHECK(baseComponent.m_HAlign == pig::ui::EHAlignType::eLeft);
					CHECK(baseComponent.m_VAlign == pig::ui::EVAlignType::eTop);
					CHECK(bool(baseComponent.m_Parent == entt::null));
					CHECK(baseComponent.m_Spacing == glm::vec2(3.5f,4.6f));
					CHECK(baseComponent.m_Size == glm::vec2(1.2f, 2.3f));
					CHECK(baseComponent.m_UUID == pig::UUID("12345678-9abc-def0-1234-56789abcdef2"));
					CHECK(baseComponent.m_Enabled);

					CHECK(dataComponent.m_Text == "sample text");
					CHECK(dataComponent.m_Kerning == 5.7f);
					CHECK(dataComponent.m_Spacing == 6.7f);
					CHECK(dataComponent.m_Color == glm::vec4(1.4f, 2.7f, 3.4f, 4.7f));
					elementCount++;
				}
				CHECK(elementCount == 1);
			}
			SECTION("Load multilevel layout (parent)")
			{
				SECTION("single file")
				{
					layoutJson.m_LayoutFilePath = "Assets/Test/TestUIWithChildren.json";
				}
				SECTION("multiple files")
				{
					layoutJson.m_LayoutFilePath = "Assets/Test/TestUIWithChildrenSplit.json";
				}
				pig::World::Get().Update(pig::Timestep(0));
				entt::entity parentEntity = entt::null;
				{
					int elementCount = 0;
					auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent>(entt::exclude<pig::ui::TextComponent, pig::ui::ImageComponent>);
					for (auto ent : viewUI)
					{
						const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
						
						CHECK(baseComponent.m_HAlign == pig::ui::EHAlignType::eLeft);
						CHECK(baseComponent.m_VAlign == pig::ui::EVAlignType::eTop);
						CHECK(bool(baseComponent.m_Parent == entt::null));
						CHECK(baseComponent.m_Spacing == glm::vec2(34.5f, 45.6f));
						CHECK(baseComponent.m_Size == glm::vec2(12.2f, 23.3f));
						CHECK(baseComponent.m_UUID == pig::UUID("12345678-9abc-def0-1234-56789abcdef0"));
						CHECK(baseComponent.m_Enabled);
						elementCount++;
						parentEntity = ent;
					}
					CHECK(elementCount == 1);
				}

				{
					int elementCount = 0;
					auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::TextComponent>();
					for (auto ent : viewUI)
					{
						const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
						const pig::ui::TextComponent& dataComponent = viewUI.get<pig::ui::TextComponent>(ent);
						CHECK(baseComponent.m_HAlign == pig::ui::EHAlignType::eLeft);
						CHECK(baseComponent.m_VAlign == pig::ui::EVAlignType::eTop);
						CHECK(bool(baseComponent.m_Parent == parentEntity));
						CHECK(baseComponent.m_Spacing == glm::vec2(3.5f, 4.6f));
						CHECK(baseComponent.m_Size == glm::vec2(1.2f, 2.3f));
						CHECK(baseComponent.m_UUID == pig::UUID("12345678-9abc-def0-1234-56789abcdef2"));
						CHECK(baseComponent.m_Enabled);

						CHECK(dataComponent.m_Text == "sample text");
						CHECK(dataComponent.m_Kerning == 5.7f);
						CHECK(dataComponent.m_Spacing == 6.7f);
						CHECK(dataComponent.m_Color == glm::vec4(1.4f, 2.7f, 3.4f, 4.7f));
						elementCount++;
					}
					CHECK(elementCount == 1);
				}
				{
					int elementCount = 0;
					auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent, const pig::ui::ImageComponent>();
					for (auto ent : viewUI)
					{
						const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
						const pig::ui::ImageComponent& dataComponent = viewUI.get<pig::ui::ImageComponent>(ent);
						CHECK(baseComponent.m_HAlign == pig::ui::EHAlignType::eCenter);
						CHECK(baseComponent.m_VAlign == pig::ui::EVAlignType::eTop);
						CHECK(bool(baseComponent.m_Parent == parentEntity));
						CHECK(baseComponent.m_Spacing == glm::vec2(123.4f, 56.7f));
						CHECK(baseComponent.m_Size == glm::vec2(4.4f, 5.7f));
						CHECK(baseComponent.m_UUID == pig::UUID("12345678-9abc-def0-1234-56789abcdef1"));
						CHECK(baseComponent.m_Enabled);

						CHECK(!dataComponent.m_TextureHandle.IsNull());
						elementCount++;
					}
					CHECK(elementCount == 1);
				}
			}
		}
	}
}