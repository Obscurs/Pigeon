#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIControlSystem.h"

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
	const char* kContainerUUID = "11111111-1111-1111-1111-1111111111b0"; // UTControlContainer.json
	const char* kContChild0UUID = "11111111-1111-1111-1111-1111111111b1";
	const char* kContChild1UUID = "11111111-1111-1111-1111-1111111111b2";

	// UIControlSystem owns BaseComponent/ImageComponent/TextComponent: they are in its
	// addSet, created only when the system parses a layout. A test must therefore not
	// create them directly. Instead it seeds a ResourceMapSingletonComponent (added in
	// production by the ResourceManagerSystem, not by this system) that maps a layout id
	// to a UT JSON file, then fires a LoadLayoutEvent so the system itself parses the
	// JSON and deferred-adds the UI components. They are visible once this Update's
	// deferred requests are flushed.
	void LoadLayout(pg::World& world, const std::string& layoutFile)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		const pg::UUID layoutID = pg::UUID::Generate();
		pg::ecs::Entity resEnt = registry.create();
		pg::ResourceMapSingletonComponent& resources =
			registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
		resources.m_UILayoutMap[layoutID] = "Assets/UT/UI/" + layoutFile;

		pg::ecs::Entity evtEnt = registry.create();
		registry.emplace<pg::ui::LoadLayoutEvent>(evtEnt).m_UUID = layoutID;

		world.Update(pg::Timestep(0));

		// One-shot load request: drop it so later frames do not reload the layout.
		registry.destroy(evtEnt);
	}

	pg::ecs::Entity FindBaseByUUID(const char* uuid)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		const pg::UUID target(uuid);
		auto view = registry.view<pg::ui::BaseComponent>();
		for (pg::ecs::Entity ent : view)
		{
			if (view.get<pg::ui::BaseComponent>(ent).m_UUID == target)
				return ent;
		}
		return pg::ecs::null;
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
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity evtEnt = registry.create();
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
		pg::ecs::Entity ent = FindBaseByUUID(kImageUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(ent));

		pg::ui::UIUpdateTransformOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateTransformOneFrameComponent>(ent);
		upd.m_AnchorMin        = { 0.1f, 0.2f };
		upd.m_AnchorMax        = { 0.3f, 0.4f };
		upd.m_Pivot            = { 0.5f, 0.6f };
		upd.m_AnchoredPosition = { 15.f, 25.f };
		upd.m_Size             = { 300.f, 400.f };

		world.Update(pg::Timestep(0));

		const pg::ui::BaseComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(ent);
		CHECK(result.m_AnchorMin        == glm::vec2(0.1f, 0.2f));
		CHECK(result.m_AnchorMax        == glm::vec2(0.3f, 0.4f));
		CHECK(result.m_Pivot            == glm::vec2(0.5f, 0.6f));
		CHECK(result.m_AnchoredPosition == glm::vec2(15.f, 25.f));
		CHECK(result.m_Size             == glm::vec2(300.f, 400.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateParentOneFrameComponent sets parent entity
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateParentApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlImage.json");
		pg::ecs::Entity childEnt = FindBaseByUUID(kImageUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(childEnt));

		// A bare entity (no component) is a valid parent handle for this update.
		pg::ecs::Entity parentEnt = pg::World::GetRegistryDirect().create();

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
		pg::ecs::Entity ent = FindBaseByUUID(kImageUUID);
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
		pg::ecs::Entity ent = FindBaseByUUID(kImageUUID);
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
		pg::ecs::Entity ent = FindBaseByUUID(kImageUUID);
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
		pg::ecs::Entity ent = FindBaseByUUID(kTextUUID);
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
		pg::ecs::Entity rootEnt = FindBaseByUUID(kHierRootUUID);
		pg::ecs::Entity midEnt  = FindBaseByUUID(kHierMidUUID);
		pg::ecs::Entity leafEnt = FindBaseByUUID(kHierLeafUUID);
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
		pg::ecs::Entity ent = FindBaseByUUID(kImageUUID);
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
	// Happy path: a JSON node with a "layout" object yields a LayoutContainerComponent
	// on the parent with the parsed fields, and its children receive sequential
	// m_SiblingIndex values from their array order.
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::ParsesLayoutContainerAndSiblingIndices")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlContainer.json");

		pg::ecs::Entity parentEnt = FindBaseByUUID(kContainerUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(parentEnt));
		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::LayoutContainerComponent>(parentEnt));

		const pg::ui::LayoutContainerComponent& container =
			pg::World::GetRegistryDirect().get<pg::ui::LayoutContainerComponent>(parentEnt);
		CHECK(container.m_Type == pg::ui::ELayoutType::eGrid);
		CHECK(container.m_Columns == 3);
		CHECK(container.m_Padding == glm::vec4(10.f, 12.f, 14.f, 16.f));
		CHECK(container.m_Spacing == glm::vec2(5.f, 7.f));
		CHECK(container.m_CellSize == glm::vec2(64.f, 48.f));

		// The same node also declares a clip with a scroll offset.
		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIClipComponent>(parentEnt));
		const pg::ui::UIClipComponent& clip =
			pg::World::GetRegistryDirect().get<pg::ui::UIClipComponent>(parentEnt);
		CHECK(clip.m_ScrollOffset == glm::vec2(3.f, -9.f));

		// Children take their sibling index from their order in the JSON children array.
		pg::ecs::Entity child0 = FindBaseByUUID(kContChild0UUID);
		pg::ecs::Entity child1 = FindBaseByUUID(kContChild1UUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(child0));
		REQUIRE(pg::World::GetRegistryDirect().valid(child1));
		CHECK(pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(child0).m_SiblingIndex == 0);
		CHECK(pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(child1).m_SiblingIndex == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateClipOffsetOneFrameComponent drives the clip's scroll offset
	// at runtime (so an app can scroll a clip view).
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateClipOffsetApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		LoadLayout(world, "UTControlContainer.json");
		pg::ecs::Entity ent = FindBaseByUUID(kContainerUUID);
		REQUIRE(pg::World::GetRegistryDirect().valid(ent));
		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIClipComponent>(ent));

		pg::ui::UIUpdateClipOffsetOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateClipOffsetOneFrameComponent>(ent);
		upd.m_ScrollOffset = { 40.f, -15.f };

		world.Update(pg::Timestep(0));

		const pg::ui::UIClipComponent& result =
			pg::World::GetRegistryDirect().get<pg::ui::UIClipComponent>(ent);
		CHECK(result.m_ScrollOffset == glm::vec2(40.f, -15.f));
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
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIUpdateClipOffsetOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::LoadLayoutEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::UIDestroyOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);

		// writeSet must cover the mutable UI components
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ui::ImageComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ui::TextComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ui::UIClipComponent))) > 0);

		// addSet must include BaseComponent, ImageComponent, TextComponent, LayoutContainerComponent (deferred from JSON load)
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::ImageComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::TextComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::LayoutContainerComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIClipComponent))) > 0);
	}

} // namespace CatchTestsetFail
