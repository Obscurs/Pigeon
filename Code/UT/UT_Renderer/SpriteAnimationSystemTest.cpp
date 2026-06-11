#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/SetSpriteAnimationRequestOneFrameComponent.h"
#include "Pigeon/Renderer/SpriteAnimationComponent.h"
#include "Pigeon/Renderer/SpriteAnimationSystem.h"
#include "Pigeon/Renderer/SpriteComponent.h"
#include "Pigeon/Renderer/SpriteSheet.h"

namespace CatchTestsetFail
{
	namespace
	{
		// Creates an entity with a 4x4 sheet animation (4 frames per row, configurable speed) and a
		// sprite, matching how an app seeds an animated entity (which the system then modifies).
		pg::ecs::Entity MakeAnimatedSprite(int row, int column, float frameDuration, bool playing)
		{
			pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
			pg::SpriteAnimationComponent& animation = pg::World::GetRegistryDirect().emplace<pg::SpriteAnimationComponent>(ent);
			animation.m_Sheet = pg::SpriteSheet(4, 4);
			animation.m_FrameCount = 4;
			animation.m_FrameDuration = frameDuration;
			animation.m_Row = row;
			animation.m_Column = column;
			animation.m_Playing = playing;
			animation.m_Elapsed = 0.f;
			pg::World::GetRegistryDirect().emplace<pg::SpriteComponent>(ent);
			return ent;
		}
	}

	// ---------------------------------------------------------------------------
	// Happy path: once the frame duration elapses the active column advances and
	// the sprite's UV rectangle matches the new cell of the active row.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteAnimationSystem::AdvancesFrameAfterFrameDuration")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SpriteAnimationSystem>());

		pg::ecs::Entity ent = MakeAnimatedSprite(1, 0, 0.1f, true);

		world.Update(pg::Timestep(100)); // 0.1s

		const pg::SpriteAnimationComponent& animation = pg::World::GetRegistryDirect().get<pg::SpriteAnimationComponent>(ent);
		const pg::SpriteComponent& sprite = pg::World::GetRegistryDirect().get<pg::SpriteComponent>(ent);
		CHECK(animation.m_Column == 1);
		CHECK(animation.m_Row == 1);
		CHECK(animation.m_Elapsed == Approx(0.f));
		CHECK(sprite.m_TexCoordsRect == pg::SpriteSheet(4, 4).GetFrameTexCoords(1, 1));
	}

	// ---------------------------------------------------------------------------
	// Before the frame duration elapses, the column holds and the elapsed time
	// accumulates; the sprite shows the current frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteAnimationSystem::HoldsFrameBeforeFrameDuration")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SpriteAnimationSystem>());

		pg::ecs::Entity ent = MakeAnimatedSprite(0, 0, 0.2f, true);

		world.Update(pg::Timestep(100)); // 0.1s < 0.2s

		const pg::SpriteAnimationComponent& animation = pg::World::GetRegistryDirect().get<pg::SpriteAnimationComponent>(ent);
		const pg::SpriteComponent& sprite = pg::World::GetRegistryDirect().get<pg::SpriteComponent>(ent);
		CHECK(animation.m_Column == 0);
		CHECK(animation.m_Elapsed == Approx(0.1f));
		CHECK(sprite.m_TexCoordsRect == pg::SpriteSheet(4, 4).GetFrameTexCoords(0, 0));
	}

	// ---------------------------------------------------------------------------
	// The column wraps back to the first frame at the end of the row.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteAnimationSystem::WrapsAtEndOfRow")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SpriteAnimationSystem>());

		pg::ecs::Entity ent = MakeAnimatedSprite(2, 3, 0.1f, true);

		world.Update(pg::Timestep(100)); // 0.1s -> advance past the last frame

		const pg::SpriteAnimationComponent& animation = pg::World::GetRegistryDirect().get<pg::SpriteAnimationComponent>(ent);
		const pg::SpriteComponent& sprite = pg::World::GetRegistryDirect().get<pg::SpriteComponent>(ent);
		CHECK(animation.m_Column == 0);
		CHECK(sprite.m_TexCoordsRect == pg::SpriteSheet(4, 4).GetFrameTexCoords(0, 2));
	}

	// ---------------------------------------------------------------------------
	// A request with m_Playing == false stops playback and resets to the idle
	// frame (column 0); no advancement happens even though time passed.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteAnimationSystem::PausedResetsToIdleFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SpriteAnimationSystem>());

		pg::ecs::Entity ent = MakeAnimatedSprite(1, 2, 0.1f, true);
		pg::World::GetRegistryDirect().get<pg::SpriteAnimationComponent>(ent).m_Elapsed = 0.05f;
		pg::SetSpriteAnimationRequestOneFrameComponent& request = pg::World::GetRegistryDirect().emplace<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		request.m_SetRow = false;
		request.m_Playing = false;

		world.Update(pg::Timestep(100));

		const pg::SpriteAnimationComponent& animation = pg::World::GetRegistryDirect().get<pg::SpriteAnimationComponent>(ent);
		const pg::SpriteComponent& sprite = pg::World::GetRegistryDirect().get<pg::SpriteComponent>(ent);
		CHECK(animation.m_Playing == false);
		CHECK(animation.m_Column == 0);
		CHECK(animation.m_Elapsed == Approx(0.f));
		CHECK(animation.m_Row == 1);
		CHECK(sprite.m_TexCoordsRect == pg::SpriteSheet(4, 4).GetFrameTexCoords(0, 1));
	}

	// ---------------------------------------------------------------------------
	// A request with m_SetRow switches the active animation row.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteAnimationSystem::RequestSwitchesRow")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SpriteAnimationSystem>());

		pg::ecs::Entity ent = MakeAnimatedSprite(0, 0, 0.2f, true);
		pg::SetSpriteAnimationRequestOneFrameComponent& request = pg::World::GetRegistryDirect().emplace<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		request.m_SetRow = true;
		request.m_Row = 2;
		request.m_Playing = true;

		world.Update(pg::Timestep(50)); // 0.05s < 0.2s, isolates the row switch

		const pg::SpriteAnimationComponent& animation = pg::World::GetRegistryDirect().get<pg::SpriteAnimationComponent>(ent);
		const pg::SpriteComponent& sprite = pg::World::GetRegistryDirect().get<pg::SpriteComponent>(ent);
		CHECK(animation.m_Row == 2);
		CHECK(animation.m_Column == 0);
		CHECK(animation.m_Playing == true);
		CHECK(sprite.m_TexCoordsRect == pg::SpriteSheet(4, 4).GetFrameTexCoords(0, 2));
	}

	// ---------------------------------------------------------------------------
	// A request without m_SetRow leaves the active row unchanged, even if it
	// carries a row value.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteAnimationSystem::RequestWithoutSetRowKeepsRow")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SpriteAnimationSystem>());

		pg::ecs::Entity ent = MakeAnimatedSprite(3, 0, 0.2f, true);
		pg::SetSpriteAnimationRequestOneFrameComponent& request = pg::World::GetRegistryDirect().emplace<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		request.m_SetRow = false;
		request.m_Row = 99;
		request.m_Playing = true;

		world.Update(pg::Timestep(50));

		const pg::SpriteAnimationComponent& animation = pg::World::GetRegistryDirect().get<pg::SpriteAnimationComponent>(ent);
		CHECK(animation.m_Row == 3);
	}

	// ---------------------------------------------------------------------------
	// Guard: a sprite without an animation component is never touched.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteAnimationSystem::SpriteWithoutAnimationUntouched")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::SpriteAnimationSystem>());

		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::SpriteComponent& sprite = pg::World::GetRegistryDirect().emplace<pg::SpriteComponent>(ent);
		sprite.m_TexCoordsRect = glm::vec4(0.25f, 0.5f, 0.75f, 1.f);

		world.Update(pg::Timestep(100));

		const pg::SpriteComponent& after = pg::World::GetRegistryDirect().get<pg::SpriteComponent>(ent);
		CHECK(after.m_TexCoordsRect == glm::vec4(0.25f, 0.5f, 0.75f, 1.f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteAnimationSystem::DeclareAccessIsCorrect")
	{
		pg::SpriteAnimationSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::SetSpriteAnimationRequestOneFrameComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::SpriteAnimationComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::SpriteComponent))) > 0);
	}
} // namespace CatchTestsetFail
