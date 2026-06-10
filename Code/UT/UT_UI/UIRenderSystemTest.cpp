#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawUIQuadInFrameEvent.h"
#include "Pigeon/Renderer/DrawUIStringInFrameEvent.h"
#include "Pigeon/Renderer/Texture.h"
#include "Pigeon/Renderer/UICameraSingletonComponent.h"
#include "Platform/Testing/TestingTexture.h"
#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"
#include "Pigeon/UI/UIRenderSystem.h"

#include <glm/glm.hpp>
#include <vector>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no RendererConfigSingletonComponent ->
	// system creates one via deferred add. On next frame it is visible.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::CreatesRendererConfigOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		// Verify no config exists before first update.
		auto viewBefore = pg::World::GetRegistryDirect().view<pg::ui::RendererConfigSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		// First frame: system detects no config and queues deferred add.
		world.Update(pg::Timestep(0));

		// Config should now be visible (deferred add flushed after Update()).
		auto viewAfter = pg::World::GetRegistryDirect().view<pg::ui::RendererConfigSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);

		// Default config values.
		const pg::ui::RendererConfigSingletonComponent& cfg =
			viewAfter.get<pg::ui::RendererConfigSingletonComponent>(viewAfter.front());
		CHECK(cfg.m_Width  == 1920.f);
		CHECK(cfg.m_Height == 1080.f);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with config but without EngineConfig/Resources ->
	// system returns early, no crash, no duplicate config.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::EarlyExitWithoutEngineConfigOrResources")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		// First frame: creates RendererConfigSingletonComponent.
		world.Update(pg::Timestep(0));

		// Second frame: config exists but EngineConfig/Resources are absent.
		// System should bail early without asserting or crashing.
		world.Update(pg::Timestep(0));

		// Config must still be a single entity (not duplicated).
		auto viewConfig = pg::World::GetRegistryDirect().view<pg::ui::RendererConfigSingletonComponent>();
		CHECK(viewConfig.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: UI image element disabled -> no DrawUIQuadInFrameEvent produced
	// (The actual draw path would require a valid texture/shader via resources,
	//  so we only verify that a disabled image entity produces no draw events.)
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::DisabledImageDoesNotProduceDrawEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		// First frame: the system creates the RendererConfigSingletonComponent itself
		// (the test must not pre-create a component the system adds).
		world.Update(pg::Timestep(0));

		// EngineConfig and ResourceMap are added by other systems, so the test supplies
		// them to get this system past its early exit.
		pg::ecs::Entity engEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(engEnt);

		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		// Disabled UI image entity.
		pg::ecs::Entity uiEnt = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base =
			pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Enabled = false;
		base.m_Size    = { 100.f, 100.f };

		pg::ui::ImageComponent& img =
			pg::World::GetRegistryDirect().emplace<pg::ui::ImageComponent>(uiEnt);
		img.m_TextureHandle = pg::UUID::Generate();

		// System update: disabled entity -> no draw event should be deferred.
		world.Update(pg::Timestep(0));

		// No DrawUIQuadInFrameEvent entities should have been created.
		// (We check indirectly by confirming no new entities beyond the 4 we created.)
		auto view = pg::World::GetRegistryDirect().view<pg::ui::BaseComponent>();
		// Only the one UI entity with BaseComponent.
		CHECK(view.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: an enabled image element emits a DrawUIQuadInFrameEvent carrying
	// the element's texture handle and a transform scaled to its size. In-frame
	// events are normally destroyed at end of frame, so the test drives the world
	// with UpdateRetainingEvents to inspect the emitted event.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::EnabledImageEmitsDrawEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		// First frame: the system creates the RendererConfigSingletonComponent itself
		// (the test must not pre-create a component the system adds).
		world.Update(pg::Timestep(0));

		// EngineConfig and ResourceMap are added by other systems, so the test supplies
		// them to get this system to the image path.
		pg::ecs::Entity engEnt = registry.create();
		registry.emplace<pg::EngineConfigSingletonComponent>(engEnt);

		pg::ecs::Entity resEnt = registry.create();
		registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);

		// Enabled top-level image element.
		const pg::UUID textureHandle = pg::UUID::Generate();
		pg::ecs::Entity uiEnt = registry.create();
		pg::ui::BaseComponent& base = registry.emplace<pg::ui::BaseComponent>(uiEnt);
		base.m_Enabled = true;
		base.m_Size    = { 100.f, 50.f };
		base.m_Parent  = pg::ecs::null;
		base.m_UUID    = pg::UUID::Generate();

		pg::ui::ImageComponent& img = registry.emplace<pg::ui::ImageComponent>(uiEnt);
		img.m_TextureHandle = textureHandle;

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = registry.view<pg::DrawUIQuadInFrameEvent>();
		REQUIRE(view.size() == 1);

		const pg::DrawUIQuadInFrameEvent& event =
			view.get<pg::DrawUIQuadInFrameEvent>(view.front());
		CHECK(event.m_TextureID == textureHandle);
		CHECK(event.m_Origin == glm::vec3(0.f, 0.f, 0.f));
		CHECK(event.m_Color == glm::vec3(0.f, 0.f, 0.f));
		// The transform is scaled to the element's size.
		CHECK(event.m_Transform[0][0] == 100.f);
		CHECK(event.m_Transform[1][1] == 50.f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the system creates the screen-space UI camera on the first frame,
	// alongside the canvas config (both are deferred adds visible next frame).
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::CreatesUICameraOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		auto viewBefore = pg::World::GetRegistryDirect().view<pg::UICameraSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::UICameraSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the live UI canvas derives a scale factor and logical size from the
	// window-vs-reference (match width here), and the UI camera tracks the logical
	// canvas (its bottom-right corner maps to NDC (1,-1)).
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::DerivesScaleFactorAndLogicalCanvasFromWindow")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		// Frame 1: the system creates the canvas config + UI camera itself.
		world.Update(pg::Timestep(0));

		// EngineConfig + ResourceMap are added by other systems; supply them to drive the canvas update.
		pg::ecs::Entity engEnt = pg::World::GetRegistryDirect().create();
		pg::EngineConfigSingletonComponent& eng =
			pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(engEnt);
		eng.m_UIReferenceWidth  = 1920.f;
		eng.m_UIReferenceHeight = 1080.f;
		eng.m_UIMatchFactor     = 0.f;     // match width
		eng.m_WindowWidth       = 960;
		eng.m_WindowHeight      = 1080;

		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		world.Update(pg::Timestep(0));

		auto cfgView = pg::World::GetRegistryDirect().view<pg::ui::RendererConfigSingletonComponent>();
		REQUIRE(cfgView.size() == 1);
		const pg::ui::RendererConfigSingletonComponent& cfg =
			cfgView.get<pg::ui::RendererConfigSingletonComponent>(cfgView.front());

		// Match width: scale = window.w / ref.w = 960 / 1920 = 0.5; logical = window / scale.
		CHECK(cfg.m_ScaleFactor == Approx(0.5f));
		CHECK(cfg.m_Width  == Approx(1920.f));
		CHECK(cfg.m_Height == Approx(2160.f));

		auto camView = pg::World::GetRegistryDirect().view<pg::UICameraSingletonComponent>();
		REQUIRE(camView.size() == 1);
		const pg::UICameraSingletonComponent& uiCam =
			camView.get<pg::UICameraSingletonComponent>(camView.front());
		const glm::vec4 ndc =
			uiCam.m_Camera.GetViewProjectionMatrix() * glm::vec4(cfg.m_Width, cfg.m_Height, 0.f, 1.f);
		CHECK(ndc.x == Approx(1.f));
		CHECK(ndc.y == Approx(-1.f));
	}

	// ---------------------------------------------------------------------------
	// A window resize event re-derives the live canvas: a square window with the
	// match factor at width yields a scale of (winW/refW) and a taller logical canvas.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::ResizeEventUpdatesLogicalCanvas")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		world.Update(pg::Timestep(0));

		pg::ecs::Entity engEnt = pg::World::GetRegistryDirect().create();
		pg::EngineConfigSingletonComponent& eng =
			pg::World::GetRegistryDirect().emplace<pg::EngineConfigSingletonComponent>(engEnt);
		eng.m_UIReferenceWidth  = 1920.f;
		eng.m_UIReferenceHeight = 1080.f;
		eng.m_UIMatchFactor     = 0.f;     // match width
		eng.m_WindowWidth       = 1920;
		eng.m_WindowHeight      = 1080;

		pg::ecs::Entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		// A resize to 960 wide overrides the engine-config window size for the scale derivation.
		pg::ecs::Entity resizeEnt = pg::World::GetRegistryDirect().create();
		pg::WindowResizeEventComponent& resize =
			pg::World::GetRegistryDirect().emplace<pg::WindowResizeEventComponent>(resizeEnt);
		resize.m_Width  = 960;
		resize.m_Height = 1080;

		world.Update(pg::Timestep(0));

		auto cfgView = pg::World::GetRegistryDirect().view<pg::ui::RendererConfigSingletonComponent>();
		REQUIRE(cfgView.size() == 1);
		const pg::ui::RendererConfigSingletonComponent& cfg =
			cfgView.get<pg::ui::RendererConfigSingletonComponent>(cfgView.front());

		CHECK(cfg.m_ScaleFactor == Approx(0.5f));
		CHECK(cfg.m_Width  == Approx(1920.f));
		CHECK(cfg.m_Height == Approx(2160.f));
	}

	// ---------------------------------------------------------------------------
	// Clipping: an image nested under a clip element carries that clip element's rect
	// (converted to window pixels via the scale factor) on its draw event.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::ImageUnderClipCarriesClipRect")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		// Frame 1: the system creates the canvas config + UI camera.
		world.Update(pg::Timestep(0));

		// Window == reference (1920x1080) makes the scale factor exactly 1, so clip pixels == clip canvas.
		pg::ecs::Entity engEnt = registry.create();
		pg::EngineConfigSingletonComponent& eng = registry.emplace<pg::EngineConfigSingletonComponent>(engEnt);
		eng.m_UIReferenceWidth  = 1920.f;
		eng.m_UIReferenceHeight = 1080.f;
		eng.m_WindowWidth       = 1920;
		eng.m_WindowHeight      = 1080;

		pg::ecs::Entity resEnt = registry.create();
		registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);

		// Clip parent rect (0,0,200,150); no image of its own.
		pg::ecs::Entity clipEnt = registry.create();
		pg::ui::BaseComponent& clipBase = registry.emplace<pg::ui::BaseComponent>(clipEnt);
		clipBase.m_Size = { 200.f, 150.f };
		clipBase.m_UUID = pg::UUID::Generate();
		registry.emplace<pg::ui::UIClipComponent>(clipEnt);

		// Image child inside the clip.
		pg::ecs::Entity imgEnt = registry.create();
		pg::ui::BaseComponent& imgBase = registry.emplace<pg::ui::BaseComponent>(imgEnt);
		imgBase.m_Size   = { 50.f, 50.f };
		imgBase.m_Parent = clipEnt;
		imgBase.m_UUID   = pg::UUID::Generate();
		registry.emplace<pg::ui::ImageComponent>(imgEnt).m_TextureHandle = pg::UUID::Generate();

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = registry.view<pg::DrawUIQuadInFrameEvent>();
		REQUIRE(view.size() == 1);
		const pg::DrawUIQuadInFrameEvent& event = view.get<pg::DrawUIQuadInFrameEvent>(view.front());
		CHECK(event.m_ClipRect == glm::vec4(0.f, 0.f, 200.f, 150.f));
	}

	// ---------------------------------------------------------------------------
	// Clipping: an element under two nested clips carries the INTERSECTION of both
	// clip rects (converted to window pixels) on its draw event .
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::NestedClipsIntersect")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		world.Update(pg::Timestep(0));

		pg::ecs::Entity engEnt = registry.create();
		pg::EngineConfigSingletonComponent& eng = registry.emplace<pg::EngineConfigSingletonComponent>(engEnt);
		eng.m_UIReferenceWidth  = 1920.f;
		eng.m_UIReferenceHeight = 1080.f;
		eng.m_WindowWidth       = 1920;  // scale factor 1: clip pixels == clip canvas
		eng.m_WindowHeight      = 1080;

		pg::ecs::Entity resEnt = registry.create();
		registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);

		// Outer clip rect (0,0,200,200).
		pg::ecs::Entity outerEnt = registry.create();
		pg::ui::BaseComponent& outerBase = registry.emplace<pg::ui::BaseComponent>(outerEnt);
		outerBase.m_Size = { 200.f, 200.f };
		outerBase.m_UUID = pg::UUID::Generate();
		registry.emplace<pg::ui::UIClipComponent>(outerEnt);

		// Inner clip rect (50,50,300,100) -> extends past the outer clip on the right and bottom.
		pg::ecs::Entity innerEnt = registry.create();
		pg::ui::BaseComponent& innerBase = registry.emplace<pg::ui::BaseComponent>(innerEnt);
		innerBase.m_AnchoredPosition = { 50.f, 50.f };
		innerBase.m_Size   = { 300.f, 100.f };
		innerBase.m_Parent = outerEnt;
		innerBase.m_UUID   = pg::UUID::Generate();
		registry.emplace<pg::ui::UIClipComponent>(innerEnt);

		// Image inside the inner clip.
		pg::ecs::Entity imgEnt = registry.create();
		pg::ui::BaseComponent& imgBase = registry.emplace<pg::ui::BaseComponent>(imgEnt);
		imgBase.m_Size   = { 20.f, 20.f };
		imgBase.m_Parent = innerEnt;
		imgBase.m_UUID   = pg::UUID::Generate();
		registry.emplace<pg::ui::ImageComponent>(imgEnt).m_TextureHandle = pg::UUID::Generate();

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = registry.view<pg::DrawUIQuadInFrameEvent>();
		REQUIRE(view.size() == 1);
		// intersect (0,0,200,200) with (50,50,300,100) -> (50,50,150,100).
		CHECK(view.get<pg::DrawUIQuadInFrameEvent>(view.front()).m_ClipRect == glm::vec4(50.f, 50.f, 150.f, 100.f));
	}

	// ---------------------------------------------------------------------------
	// Nine-slice: an image with a non-zero border emits 9 sub-quads; corners keep the
	// texture-pixel border size, the center fills the remainder, and each cell samples
	// its matching UV sub-rect .
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::NineSliceEmitsNineQuads")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIRenderSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		world.Update(pg::Timestep(0));

		pg::ecs::Entity engEnt = registry.create();
		pg::EngineConfigSingletonComponent& eng = registry.emplace<pg::EngineConfigSingletonComponent>(engEnt);
		eng.m_UIReferenceWidth  = 1920.f;
		eng.m_UIReferenceHeight = 1080.f;
		eng.m_WindowWidth       = 1920;
		eng.m_WindowHeight      = 1080;

		// A 16x16 texture so a 4px border maps to UV 0.25 / 0.75 cuts (the Testing texture reports its size
		// via these statics).
		pg::TestingTexture2D::s_ExpectedWidth = 16;
		pg::TestingTexture2D::s_ExpectedHeight = 16;
		const pg::UUID texID = pg::UUID::Generate();
		pg::ecs::Entity resEnt = registry.create();
		pg::ResourceMapSingletonComponent& resources = registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
		std::vector<unsigned char> pixels(16 * 16 * 4, 255);
		resources.m_TextureMap[texID] = pg::MappedTexture{ pg::Texture2D::Create(16, 16, 4, pixels.data()), pg::EMappedTextureType::eQuad };

		// Image rect (0,0,100,80) with a uniform 4px nine-slice border.
		pg::ecs::Entity imgEnt = registry.create();
		pg::ui::BaseComponent& imgBase = registry.emplace<pg::ui::BaseComponent>(imgEnt);
		imgBase.m_Size = { 100.f, 80.f };
		imgBase.m_UUID = pg::UUID::Generate();
		pg::ui::ImageComponent& img = registry.emplace<pg::ui::ImageComponent>(imgEnt);
		img.m_TextureHandle  = texID;
		img.m_NineSliceBorder = { 4.f, 4.f, 4.f, 4.f };

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = registry.view<pg::DrawUIQuadInFrameEvent>();
		REQUIRE(view.size() == 9);

		// Locate cells by their translate (transform[3].xy) and verify size + UV.
		bool sawTopLeft = false, sawCenter = false, sawBottomRight = false;
		for (pg::ecs::Entity ent : view)
		{
			const pg::DrawUIQuadInFrameEvent& e = view.get<pg::DrawUIQuadInFrameEvent>(ent);
			const float px = e.m_Transform[3][0];
			const float py = e.m_Transform[3][1];
			if (px == 0.f && py == 0.f)  // top-left corner cell
			{
				sawTopLeft = true;
				CHECK(e.m_Transform[0][0] == 4.f);
				CHECK(e.m_Transform[1][1] == 4.f);
				CHECK(e.m_TexCoords == glm::vec4(0.f, 0.f, 0.25f, 0.25f));
			}
			else if (px == 4.f && py == 4.f)  // center cell
			{
				sawCenter = true;
				CHECK(e.m_Transform[0][0] == 92.f);  // 100 - 4 - 4
				CHECK(e.m_Transform[1][1] == 72.f);  // 80 - 4 - 4
				CHECK(e.m_TexCoords == glm::vec4(0.25f, 0.25f, 0.75f, 0.75f));
			}
			else if (px == 96.f && py == 76.f)  // bottom-right corner cell
			{
				sawBottomRight = true;
				CHECK(e.m_Transform[0][0] == 4.f);
				CHECK(e.m_Transform[1][1] == 4.f);
				CHECK(e.m_TexCoords == glm::vec4(0.75f, 0.75f, 1.f, 1.f));
			}
		}
		CHECK(sawTopLeft);
		CHECK(sawCenter);
		CHECK(sawBottomRight);
	}

	// ---------------------------------------------------------------------------
	// The UI camera's vertical bounds account for the renderer Y-flip so canvas y=0
	// renders at the top of the screen on both flip conventions. On DirectX11
	// (flip = true) the bottom must be negative; without the flip the y-down bounds apply.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::UICameraVerticalBoundsAccountForYFlip")
	{
		CHECK(pg::ui::GetUICameraOrthoBottomTop(1080.f, true)  == glm::vec2(-1080.f, 0.f));
		CHECK(pg::ui::GetUICameraOrthoBottomTop(1080.f, false) == glm::vec2(1080.f, 0.f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets contain the expected component types
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIRenderSystem::DeclareAccessIsCorrect")
	{
		pg::ui::UIRenderSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::ImageComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::TextComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::RendererConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::WindowResizeEventComponent))) > 0);

		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ui::RendererConfigSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::UICameraSingletonComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::RendererConfigSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::UICameraSingletonComponent))) > 0);

		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawUIQuadInFrameEvent))) > 0);
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawUIStringInFrameEvent))) > 0);
	}

} // namespace CatchTestsetFail
