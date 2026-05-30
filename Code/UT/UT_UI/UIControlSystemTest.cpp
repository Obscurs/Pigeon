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
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		// Create a UI entity with an update transform component but NO resource map.
		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base = pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(ent);
		base.m_Size = { 100.f, 200.f };

		pig::ui::UIUpdateTransformOneFrameComponent& upd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateTransformOneFrameComponent>(ent);
		upd.m_Size = { 999.f, 888.f };

		world.Update(pig::Timestep(0));

		// Size must be unchanged because system bails early without resource map.
		const pig::ui::BaseComponent& baseAfter =
			pig::World::GetRegistryDirect().get<pig::ui::BaseComponent>(ent);
		CHECK(baseAfter.m_Size == glm::vec2(100.f, 200.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateTransformOneFrameComponent copies fields into BaseComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateTransformApplied")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		// Resource map singleton required by the system guard.
		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base = pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(ent);
		base.m_Size    = { 100.f, 200.f };
		base.m_Spacing = { 5.f,   10.f  };
		base.m_HAlign  = pig::ui::EHAlignType::eCenter;
		base.m_VAlign  = pig::ui::EVAlignType::eCenter;

		pig::ui::UIUpdateTransformOneFrameComponent& upd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateTransformOneFrameComponent>(ent);
		upd.m_Size    = { 300.f, 400.f };
		upd.m_Spacing = { 15.f,  25.f  };
		upd.m_HAlign  = pig::ui::EHAlignType::eLeft;
		upd.m_VAlign  = pig::ui::EVAlignType::eBottom;

		world.Update(pig::Timestep(0));

		const pig::ui::BaseComponent& result =
			pig::World::GetRegistryDirect().get<pig::ui::BaseComponent>(ent);
		CHECK(result.m_Size    == glm::vec2(300.f, 400.f));
		CHECK(result.m_Spacing == glm::vec2(15.f, 25.f));
		CHECK(result.m_HAlign  == pig::ui::EHAlignType::eLeft);
		CHECK(result.m_VAlign  == pig::ui::EVAlignType::eBottom);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateParentOneFrameComponent sets parent entity
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateParentApplied")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity parentEnt = pig::World::GetRegistryDirect().create();
		entt::entity childEnt  = pig::World::GetRegistryDirect().create();

		pig::ui::BaseComponent& base = pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(childEnt);
		base.m_Parent = entt::null;

		pig::ui::UIUpdateParentOneFrameComponent& upd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateParentOneFrameComponent>(childEnt);
		upd.m_Parent = parentEnt;

		world.Update(pig::Timestep(0));

		const pig::ui::BaseComponent& result =
			pig::World::GetRegistryDirect().get<pig::ui::BaseComponent>(childEnt);
		CHECK(result.m_Parent == parentEnt);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateEnableOneFrameComponent sets enabled flag
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateEnableApplied")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base = pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(ent);
		base.m_Enabled = true;

		pig::ui::UIUpdateEnableOneFrameComponent& upd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateEnableOneFrameComponent>(ent);
		upd.m_Enabled = false;

		world.Update(pig::Timestep(0));

		const pig::ui::BaseComponent& result =
			pig::World::GetRegistryDirect().get<pig::ui::BaseComponent>(ent);
		CHECK(result.m_Enabled == false);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateUUIDOneFrameComponent sets UUID
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateUUIDApplied")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(ent);

		pig::UUID newID = pig::UUID::Generate();
		pig::ui::UIUpdateUUIDOneFrameComponent& upd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateUUIDOneFrameComponent>(ent);
		upd.m_UUID = newID;

		world.Update(pig::Timestep(0));

		const pig::ui::BaseComponent& result =
			pig::World::GetRegistryDirect().get<pig::ui::BaseComponent>(ent);
		CHECK(result.m_UUID == newID);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateImageUUIDOneFrameComponent updates texture handle
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateImageUUIDApplied")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::UUID oldID = pig::UUID::Generate();
		pig::UUID newID = pig::UUID::Generate();

		pig::ui::ImageComponent& img =
			pig::World::GetRegistryDirect().emplace<pig::ui::ImageComponent>(ent);
		img.m_TextureHandle = oldID;

		pig::ui::UIUpdateImageUUIDOneFrameComponent& upd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateImageUUIDOneFrameComponent>(ent);
		upd.m_UUID = newID;

		world.Update(pig::Timestep(0));

		const pig::ui::ImageComponent& result =
			pig::World::GetRegistryDirect().get<pig::ui::ImageComponent>(ent);
		CHECK(result.m_TextureHandle == newID);
	}

	// ---------------------------------------------------------------------------
	// Happy path: UIUpdateTextOneFrameComponent updates text fields
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::UpdateTextApplied")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::ui::TextComponent& txt =
			pig::World::GetRegistryDirect().emplace<pig::ui::TextComponent>(ent);
		txt.m_Text    = "old";
		txt.m_Color   = { 0.f, 0.f, 0.f, 1.f };
		txt.m_Kerning = 1.f;
		txt.m_Spacing = 1.f;

		pig::ui::UIUpdateTextOneFrameComponent& upd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateTextOneFrameComponent>(ent);
		upd.m_Text    = "new text";
		upd.m_Color   = { 0.2f, 0.4f, 0.6f, 0.8f };
		upd.m_Kerning = 2.5f;
		upd.m_Spacing = 3.0f;

		world.Update(pig::Timestep(0));

		const pig::ui::TextComponent& result =
			pig::World::GetRegistryDirect().get<pig::ui::TextComponent>(ent);
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
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		// root -> parent -> child hierarchy
		entt::entity rootEnt   = pig::World::GetRegistryDirect().create();
		entt::entity parentEnt = pig::World::GetRegistryDirect().create();
		entt::entity childEnt  = pig::World::GetRegistryDirect().create();

		pig::ui::BaseComponent& rootBase   = pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(rootEnt);
		pig::ui::BaseComponent& parentBase = pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(parentEnt);
		pig::ui::BaseComponent& childBase  = pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(childEnt);

		rootBase.m_Parent   = entt::null;
		parentBase.m_Parent = rootEnt;
		childBase.m_Parent  = parentEnt;

		// Request destroy of parentEnt (and its subtree: childEnt).
		pig::World::GetRegistryDirect().emplace<pig::ui::UIDestroyOneFrameComponent>(parentEnt);

		world.Update(pig::Timestep(0));

		// parentEnt and childEnt must be gone; rootEnt must survive.
		CHECK(!pig::World::GetRegistryDirect().valid(parentEnt));
		CHECK(!pig::World::GetRegistryDirect().valid(childEnt));
		CHECK(pig::World::GetRegistryDirect().valid(rootEnt));
	}

	// ---------------------------------------------------------------------------
	// Edge case: multiple update components on same entity applied in same frame
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::MultipleUpdatesInOneFrame")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::ui::UIControlSystem>());

		entt::entity resEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::ResourceMapSingletonComponent>(resEnt);

		entt::entity ent = pig::World::GetRegistryDirect().create();
		pig::ui::BaseComponent& base = pig::World::GetRegistryDirect().emplace<pig::ui::BaseComponent>(ent);
		base.m_Enabled = true;
		base.m_Size    = { 1.f, 1.f };

		entt::entity newParent = pig::World::GetRegistryDirect().create();

		// Apply both a transform update and an enable update in the same frame.
		pig::ui::UIUpdateTransformOneFrameComponent& tUpd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateTransformOneFrameComponent>(ent);
		tUpd.m_Size = { 50.f, 75.f };

		pig::ui::UIUpdateEnableOneFrameComponent& eUpd =
			pig::World::GetRegistryDirect().emplace<pig::ui::UIUpdateEnableOneFrameComponent>(ent);
		eUpd.m_Enabled = false;

		world.Update(pig::Timestep(0));

		const pig::ui::BaseComponent& result =
			pig::World::GetRegistryDirect().get<pig::ui::BaseComponent>(ent);
		CHECK(result.m_Size    == glm::vec2(50.f, 75.f));
		CHECK(result.m_Enabled == false);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared access contains expected component types
	// ---------------------------------------------------------------------------
	TEST_CASE("UI.UIControlSystem::DeclareAccessIsCorrect")
	{
		pig::ui::UIControlSystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		// readSet must contain the one-frame update components and layout event
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::UIUpdateTransformOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::UIUpdateParentOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::UIUpdateEnableOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::UIUpdateUUIDOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::UIUpdateImageUUIDOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::UIUpdateTextOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::LoadLayoutEvent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::UIDestroyOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::ui::BaseComponent))) > 0);

		// writeSet must cover the mutable UI components
		CHECK(decl.writeSet.count(std::type_index(typeid(pig::ui::BaseComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pig::ui::ImageComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pig::ui::TextComponent))) > 0);

		// addSet must include BaseComponent, ImageComponent, TextComponent (deferred from JSON load)
		CHECK(decl.addSet.count(std::type_index(typeid(pig::ui::BaseComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pig::ui::ImageComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pig::ui::TextComponent))) > 0);
	}

} // namespace CatchTestsetFail
