#include "Sandbox/CharacterControlSystem.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/SetSpriteAnimationRequestOneFrameComponent.h"
#include "Pigeon/Transform/PositionComponent.h"
#include "Sandbox/CharacterTagComponent.h"
#include "Sandbox/CharacterTransformRequestOneFrameComponent.h"

#include <glm/glm.hpp>
#include <utility>

namespace
{
	// Sprite-sheet rows used for each facing direction of the showcase character.
	constexpr int CHARACTER_ROW_DOWN = 0;
	constexpr int CHARACTER_ROW_UP = 1;
	constexpr int CHARACTER_ROW_LEFT = 2;
	constexpr int CHARACTER_ROW_RIGHT = 3;

	constexpr float CHARACTER_SPEED = 1.5f; // world units per second

	bool IsKeyHeld(const pg::InputStateSingletonComponent& input, int key)
	{
		return input.m_KeysPressed.count(key) > 0;
	}

	// Picks the facing row from the movement axes. Horizontal facing wins when both axes are held, so a
	// diagonal still shows a left/right animation; only pure vertical movement faces up/down.
	int FacingRow(float moveX, float moveY)
	{
		if (moveX > 0.f)
		{
			return CHARACTER_ROW_RIGHT;
		}
		if (moveX < 0.f)
		{
			return CHARACTER_ROW_LEFT;
		}
		if (moveY > 0.f)
		{
			return CHARACTER_ROW_UP;
		}
		return CHARACTER_ROW_DOWN;
	}
}

pg::SystemAccessDecl sbx::CharacterControlSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::InputStateSingletonComponent)),
		std::type_index(typeid(sbx::CharacterTagComponent)),
		std::type_index(typeid(pg::PositionComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::CharacterTransformRequestOneFrameComponent)),
		std::type_index(typeid(pg::SetSpriteAnimationRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::CharacterControlSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto inputView = accessor.View<const pg::InputStateSingletonComponent>();
	if (inputView.empty())
	{
		return;
	}
	const pg::InputStateSingletonComponent& input = inputView.get<const pg::InputStateSingletonComponent>(inputView.front());

	const float moveX = (IsKeyHeld(input, pg::PG_KEY_RIGHT) ? 1.f : 0.f) - (IsKeyHeld(input, pg::PG_KEY_LEFT) ? 1.f : 0.f);
	const float moveY = (IsKeyHeld(input, pg::PG_KEY_UP) ? 1.f : 0.f) - (IsKeyHeld(input, pg::PG_KEY_DOWN) ? 1.f : 0.f);
	const bool moving = (moveX != 0.f || moveY != 0.f);

	auto characterView = accessor.View<const sbx::CharacterTagComponent>();
	for (pg::ecs::Entity ent : characterView)
	{
		pg::SetSpriteAnimationRequestOneFrameComponent animationRequest;
		animationRequest.m_Playing = moving;

		if (moving)
		{
			animationRequest.m_SetRow = true;
			animationRequest.m_Row = FacingRow(moveX, moveY);

			glm::vec3 position(0.f, 0.f, 0.f);
			if (accessor.AllOf<pg::PositionComponent>(ent))
			{
				position = accessor.Get<const pg::PositionComponent>(ent).m_Position;
			}
			// moveX/moveY are screen-space intents (up = +1) and feed the facing row. World +Y renders
			// downward (the renderer flips Y on DirectX), so moving up on screen means decreasing world Y.
			const glm::vec2 direction = glm::normalize(glm::vec2(moveX, moveY));
			position.x += direction.x * CHARACTER_SPEED * ts.AsSeconds();
			position.y -= direction.y * CHARACTER_SPEED * ts.AsSeconds();

			sbx::CharacterTransformRequestOneFrameComponent transformRequest;
			transformRequest.m_Data.m_SetPosition = true;
			transformRequest.m_Data.m_Position = position;
			accessor.EmplaceOneframe<sbx::CharacterTransformRequestOneFrameComponent>(ent, std::move(transformRequest));
		}

		accessor.EmplaceOneframe<pg::SetSpriteAnimationRequestOneFrameComponent>(ent, std::move(animationRequest));
	}
}
