#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/ResourceMapSingletonComponent.h>
#include <Pigeon/UI/UIComponents.h>
#include <Pigeon/UI/UIControlSystem.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard condition: no ResourceMapSingletonComponent -> system does nothing
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::DoesNothingWithoutResourceMap")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		// Create a UI entity with an update transform component but NO resource map.
		entt::entity ent = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(ent);
		base.m_Size = { 100.f, 200.f };

		pg::ui::UIUpdateTransformOneFrameComponent& upd =
			pg::World::GetRegistryDirect().emplace<pg::ui::UIUpdateTransformOneFrameComponent>(ent);
		upd.m_Size = { 999.f, 888.f };

		world.Update(pg::Timestep(0));

		// Size must be unchanged because system bails early without resource map.
		const pg::ui::BaseComponent& baseAfter =
			pg::World::GetRegistryDirect().get<pg::ui::BaseComponent>(ent);
		CHECK(baseAfter.m_Size == glm::vec2(100.f, 200.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateTransformOneFrameComponent copies fields into BaseComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateTransformApplied")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		// Resource map singleton required by the system guard.
		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(ent);
		base.m_Size    = { 100.f, 200.f };
		base.m_Spacing = { 5.f,   10.f  };
		base.m_HAlign  = pg::ui::EHAlignType::eCenter;
		base.m_VAlign  = pg::ui::EVAlignType::eCenter;

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

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		entt::entity parentEnt = pg::World::GetRegistryDirect().create();
		entt::entity childEnt  = pg::World::GetRegistryDirect().create();

		pg::ui::BaseComponent& base = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(childEnt);
		base.m_Parent = entt::null;

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

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(ent);
		base.m_Enabled = true;

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

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(ent);

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

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pg::World::GetRegistryDirect().create();
		pg::UUID oldID = pg::UUID::Generate();
		pg::UUID newID = pg::UUID::Generate();

		pg::ui::ImageComponent& img =
			pg::World::GetRegistryDirect().emplace<pg::ui::ImageComponent>(ent);
		img.m_TextureHandle = oldID;

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

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pg::World::GetRegistryDirect().create();
		pg::ui::TextComponent& txt =
			pg::World::GetRegistryDirect().emplace<pg::ui::TextComponent>(ent);
		txt.m_Text    = "old";
		txt.m_Color   = { 0.f, 0.f, 0.f, 1.f };
		txt.m_Kerning = 1.f;
		txt.m_Spacing = 1.f;

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

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		// root -> parent -> child hierarchy
		entt::entity rootEnt   = pg::World::GetRegistryDirect().create();
		entt::entity parentEnt = pg::World::GetRegistryDirect().create();
		entt::entity childEnt  = pg::World::GetRegistryDirect().create();

		pg::ui::BaseComponent& rootBase   = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(rootEnt);
		pg::ui::BaseComponent& parentBase = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(parentEnt);
		pg::ui::BaseComponent& childBase  = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(childEnt);

		rootBase.m_Parent   = entt::null;
		parentBase.m_Parent = rootEnt;
		childBase.m_Parent  = parentEnt;

		// Request destroy of parentEnt (and its subtree: childEnt).
		pg::World::GetRegistryDirect().emplace<pg::ui::UIDestroyOneFrameComponent>(parentEnt);

		world.Update(pg::Timestep(0));

		// parentEnt and childEnt must be gone; rootEnt must survive.
		CHECK(!pg::World::GetRegistryDirect().valid(parentEnt));
		CHECK(!pg::World::GetRegistryDirect().valid(childEnt));
		CHECK(pg::World::GetRegistryDirect().valid(rootEnt));
	}

	// ---------------------------------------------------------------------------
	// Edge case: multiple update components on same entity applied in same frame
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::MultipleUpdatesInOneFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ui::UIControlSystem>());

		entt::entity resEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pg::World::GetRegistryDirect().create();
		pg::ui::BaseComponent& base = pg::World::GetRegistryDirect().emplace<pg::ui::BaseComponent>(ent);
		base.m_Enabled = true;
		base.m_Size    = { 1.f, 1.f };

		entt::entity newParent = pg::World::GetRegistryDirect().create();

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
