#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawStringInFrameEvent.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Texture.h"
#include "Sandbox/LabelComponent.h"
#include "Sandbox/TextRenderSystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace
{
	// ResourceMapSingletonComponent is added in production by ResourceManagerSystem (a different
	// system), so the test seeds it directly. The render bridge treats the font as an opaque
	// shared pointer (it only forwards it into the draw event), so a non-loadable font path is
	// used on purpose: it yields a valid, non-null Font object without running the MSDF atlas
	// generation, which keeps this test independent of global render state set up by other tests.
	pg::S_Ptr<pg::Font> SeedResourceMapWithFont(pg::ecs::Registry& registry, const pg::UUID& fontID)
	{
		pg::ecs::Entity resEnt = registry.create();
		pg::ResourceMapSingletonComponent& resources = registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
		pg::TextureData textureData;
		pg::S_Ptr<pg::Font> font = std::make_shared<pg::Font>("Assets/UT/Fonts/__missing_for_test__.ttf", textureData);
		resources.m_FontMap[fontID] = font;
		return font;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: a label with a resolvable font emits a matching draw string event.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.TextRenderSystem::EmitsMatchingStringEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TextRenderSystem>());

		const pg::UUID fontID = pg::UUID::Generate();
		pg::S_Ptr<pg::Font> font = SeedResourceMapWithFont(pg::World::GetRegistryDirect(), fontID);

		// LabelComponent is added in production by SceneSetupSystem (a different system).
		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		sbx::LabelComponent& label = pg::World::GetRegistryDirect().emplace<sbx::LabelComponent>(ent);
		label.m_Transform = glm::translate(glm::mat4(1.f), glm::vec3(3.f, 4.f, 0.f));
		label.m_Text = "hello";
		label.m_FontID = fontID;
		label.m_Color = { 0.1f, 0.2f, 0.3f, 0.4f };
		label.m_Kerning = 0.2f;
		label.m_Linespacing = 0.3f;

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<const pg::DrawStringInFrameEvent>();
		REQUIRE(view.size() == 1);
		const pg::DrawStringInFrameEvent& event = view.get<const pg::DrawStringInFrameEvent>(view.front());
		CHECK(event.m_String == "hello");
		CHECK(event.m_Font == font);
		CHECK(event.m_Transform == label.m_Transform);
		CHECK(event.m_Color == label.m_Color);
		CHECK(std::fabs(event.m_Kerning - 0.2f) < 1e-4f);
		CHECK(std::fabs(event.m_Linespacing - 0.3f) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Guard: no ResourceMapSingletonComponent -> no draw events.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.TextRenderSystem::NoOpWithoutResourceMap")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TextRenderSystem>());

		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::LabelComponent>(ent).m_FontID = pg::UUID::Generate();

		world.UpdateRetainingEvents(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<const pg::DrawStringInFrameEvent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Edge case: a label whose font is not in the map is skipped (no event).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.TextRenderSystem::SkipsLabelWithUnresolvedFont")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TextRenderSystem>());

		SeedResourceMapWithFont(pg::World::GetRegistryDirect(), pg::UUID::Generate());

		// Label references a font id that is not present in the resource map.
		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::LabelComponent>(ent).m_FontID = pg::UUID::Generate();

		world.UpdateRetainingEvents(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<const pg::DrawStringInFrameEvent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.TextRenderSystem::DeclareAccessIsCorrect")
	{
		sbx::TextRenderSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(sbx::LabelComponent))) > 0);
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawStringInFrameEvent))) > 0);
	}

} // namespace CatchTestsetFail
