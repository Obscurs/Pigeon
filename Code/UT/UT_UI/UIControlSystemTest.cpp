#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/ResourceMapSingletonComponent.h>
#include <Pigeon/UI/UIComponents.h>
#include <Pigeon/UI/UIControlSystem.h>

#include <string>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	// UUIDs embedded in the UT layout JSON files.
	const char* kImageUUID     = "11111111-1111-1111-1111-111111111101"; // UTControlImage.json
	const char* kTextUUID      = "11111111-1111-1111-1111-111111111102"; // UTControlText.json
	const char* kHierRootUUID  = "11111111-1111-1111-1111-1111111111a0"; // UTControlHierarchy.json
	const char* kHierMidUUID   = "11111111-1111-1111-1111-1111111111a1";
	const char* kHierLeafUUID  = "11111111-1111-1111-1111-1111111111a2";

	// UIControlSystem owns BaseComponent/ImageComponent/TextComponent: they are in its
	// addSet, created only when the system parses a layout. A test must therefore not
	// create them directly. Instead it seeds a ResourceMapSingletonComponent (added in
	// production by the ResourceManagerSystem, not by this system) that maps a layout id
	// to a UT JSON file, then fires a LoadLayoutEvent so the system itself parses the
	// JSON and deferred-adds the UI components. They are visible once this Update's
	// deferred requests are flushed.
	void LoadLayout(pg::World& world, const std::string& layoutFile)
	{
		entt::registry& registry = pg::World::GetRegistryDirect();

		const pg::UUID layoutID = pg::UUID::Generate();
		entt::entity resEnt = registry.create();
		pg::ResourceMapSingletonComponent& resources =
			registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
		resources.m_UILayoutMap[layoutID] = "Assets/UT/UI/" + layoutFile;

		entt::entity evtEnt = registry.create();
		registry.emplace<pg::ui::LoadLayoutEvent>(evtEnt).m_UUID = layoutID;

		world.Update(pg::Timestep(0));

		// One-shot load request: drop it so later frames do not reload the layout.
		registry.destroy(evtEnt);
	}

	entt::entity FindBaseByUUID(const char* uuid)
	{
		entt::registry& registry = pg::World::GetRegistryDirect();
		const pg::UUID target(uuid);
		auto view = registry.view<pg::ui::BaseComponent>();
		for (entt::entity ent : view)
		{
			if (view.get<pg::ui::BaseComponent>(ent).m_UUID == target)
				return ent;
		}
		return entt::null;
	}
} // namespace

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard condition: no ResourceMapSingletonComponent -> system bails out early,
	// so a load request produces no UI components.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::DoesNothingWithoutResourceMap")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		// Load request, but no ResourceMapSingletonComponent present.
		entt::registry& registry = pg::World::GetRegistryDirect();
		entt::entity evtEnt = registry.create();
		registry.emplace<pg::ui::LoadLayoutEvent>(evtEnt).m_UUID = pg::UUID::Generate();

		world.Update(pg::Timestep(0));

		// System bailed before parsing -> no BaseComponent created.
		auto view = registry.view<pg::ui::BaseComponent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateTransformOneFrameComponent copies fields into BaseComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateTransformApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlImage.json");
		entt::entity ent = FindBaseByUUID(kImageUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(ent));

		pg::ui::UIUpdateTransformOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateTransformOneFrameComponent>(ent);
		upd.m_Size    = { 300.f, 400.f };
		upd.m_Spacing = { 15.f,  25.f  };
		upd.m_HAlign  = pg::ui::EHAlignType::eLeft;
		upd.m_VAlign  = pg::ui::EVAlignType::eBottom;

		world.Update(pg::Timestep(0));

		const pg::ui::BaseComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(ent);
		CHECK(result.m_Size    == glm::vec2(300.f, 400.f));
		CHECK(result.m_Spacing == glm::vec2(15.f, 25.f));
		CHECK(result.m_HAlign  == pg::ui::EHAlignType::eLeft);
		CHECK(result.m_VAlign  == pg::ui::EVAlignType::eBottom);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateParentOneFrameComponent sets parent entity
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateParentApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlImage.json");
		entt::entity childEnt = FindBaseByUUID(kImageUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(childEnt));

		// A bare entity (no component) is a valid parent handle for this update.
		entt::entity parentEnt = pg::World::GetRegistryDirect().create();

		pg::ui::UIUpdateParentOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateParentOneFrameComponent>(childEnt);
		upd.m_Parent = parentEnt;

		world.Update(pg::Timestep(0));

		const pg::ui::BaseComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(childEnt);
		CHECK(result.m_Parent == parentEnt);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateEnableOneFrameComponent sets enabled flag
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateEnableApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlImage.json");
		entt::entity ent = FindBaseByUUID(kImageUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(ent));

		pg::ui::UIUpdateEnableOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateEnableOneFrameComponent>(ent);
		upd.m_Enabled = false;

		world.Update(pg::Timestep(0));

		const pg::ui::BaseComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(ent);
		CHECK(result.m_Enabled == false);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateUUIDOneFrameComponent sets UUID
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateUUIDApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlImage.json");
		entt::entity ent = FindBaseByUUID(kImageUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(ent));

		pg::UUID newID = pg::UUID::Generate();
		pg::ui::UIUpdateUUIDOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateUUIDOneFrameComponent>(ent);
		upd.m_UUID = newID;

		world.Update(pg::Timestep(0));

		const pg::ui::BaseComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(ent);
		CHECK(result.m_UUID == newID);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateImageUUIDOneFrameComponent updates texture handle
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateImageUUIDApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlImage.json");
		entt::entity ent = FindBaseByUUID(kImageUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(ent));
		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::ImageComponent>(ent));

		pg::UUID newID = pg::UUID::Generate();
		pg::ui::UIUpdateImageUUIDOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateImageUUIDOneFrameComponent>(ent);
		upd.m_UUID = newID;

		world.Update(pg::Timestep(0));

		const pg::ui::ImageComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::ImageComponent>(ent);
		CHECK(result.m_TextureHandle == newID);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateTextOneFrameComponent updates text fields
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateTextApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlText.json");
		entt::entity ent = FindBaseByUUID(kTextUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(ent));
		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::TextComponent>(ent));

		pg::ui::UIUpdateTextOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateTextOneFrameComponent>(ent);
		upd.m_Text    = "new text";
		upd.m_Color   = { 0.2f, 0.4f, 0.6f, 0.8f };
		upd.m_Kerning = 2.5f;
		upd.m_Spacing = 3.0f;

		world.Update(pg::Timestep(0));

		const pg::ui::TextComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::TextComponent>(ent);
		CHECK(result.m_Text    == "new text");
		CHECK(result.m_Color   == glm::vec4(0.2f, 0.4f, 0.6f, 0.8f));
		CHECK(FLOAT_EQ(result.m_Kerning, 2.5f));
		CHECK(FLOAT_EQ(result.m_Spacing, 3.0f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIDestroyOneFrameComponent destroys entity and its children
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::DestroyEntityAndChildren")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		// Layout is a root -> mid -> leaf hierarchy created by the system.
		LoadLayout(world, "UTControlHierarchy.json");
		entt::entity rootEnt = FindBaseByUUID(kHierRootUUID);
		entt::entity midEnt  = FindBaseByUUID(kHierMidUUID);
		entt::entity leafEnt = FindBaseByUUID(kHierLeafUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(rootEnt));
		REQUIRE(pg::World::GetRegistryDirect().valid(midEnt));
		REQUIRE(pg::World::GetRegistryDirect().valid(leafEnt));

		// Request destroy of the mid element (and its subtree: the leaf).
		pg::World::GetRegistryDirect().emplace<pg::ui::UIDestroyOneFrameComponent>(midEnt);

		world.Update(pg::Timestep(0));

		// mid and leaf must be gone; root must survive.
		CHECK(!pg::World::GetRegistryDirect().valid(midEnt));
		CHECK(!pg::World::GetRegistryDirect().valid(leafEnt));
		CHECK(pg::World::GetRegistryDirect().valid(rootEnt));
	}

	// ---------------------------------------------------------------------------
	// Edge case: multiple update components on same entity applied in same frame
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::MultipleUpdatesInOneFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlImage.json");
		entt::entity ent = FindBaseByUUID(kImageUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(ent));

		// Apply both a transform update and an enable update in the same frame.
		pg::ui::UIUpdateTransformOneFrameComponent& tUpd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateTransformOneFrameComponent>(ent);
		tUpd.m_Size = { 50.f, 75.f };

		pg::ui::UIUpdateEnableOneFrameComponent& eUpd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateEnableOneFrameComponent>(ent);
		eUpd.m_Enabled = false;

		world.Update(pg::Timestep(0));

		const pg::ui::BaseComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(ent);
		CHECK(result.m_Size    == glm::vec2(50.f, 75.f));
		CHECK(result.m_Enabled == false);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared access contains expected component types
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::DeclareAccessIsCorrect")
	{
		pg::ui::UIControlSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		// readSet must contain the one-frame update components and layout event
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIUpdateTransformOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIUpdateParentOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIUpdateEnableOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIUpdateUUIDOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIUpdateImageUUIDOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIUpdateTextOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::LoadLayoutEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIDestroyOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);

		// writeSet must cover the mutable UI components
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ui::ImageComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ui::TextComponent))) > 0);

		// addSet must include BaseComponent, ImageComponent, TextComponent (deferred from JSON load)
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::ImageComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::TextComponent))) > 0);
	}

} // namespace CatchTestsetFail
