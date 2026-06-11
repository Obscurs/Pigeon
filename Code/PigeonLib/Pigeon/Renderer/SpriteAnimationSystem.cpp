#include "pch.h"
#include "Pigeon/Renderer/SpriteAnimationSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/SetSpriteAnimationRequestOneFrameComponent.h"
#include "Pigeon/Renderer/SpriteAnimationComponent.h"
#include "Pigeon/Renderer/SpriteComponent.h"

#include <algorithm>

pg::SystemAccessDecl pg::SpriteAnimationSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::SetSpriteAnimationRequestOneFrameComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::SpriteAnimationComponent)),
		std::type_index(typeid(pg::SpriteComponent)),
	};
	return decl;
}

void pg::SpriteAnimationSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	const float deltaSeconds = ts.AsSeconds();

	auto view = accessor.View<pg::SpriteAnimationComponent, pg::SpriteComponent>();
	for (pg::ecs::Entity ent : view)
	{
		pg::SpriteAnimationComponent& animation = view.get<pg::SpriteAnimationComponent>(ent);
		pg::SpriteComponent& sprite = view.get<pg::SpriteComponent>(ent);

		if (accessor.AllOf<pg::SetSpriteAnimationRequestOneFrameComponent>(ent))
		{
			const pg::SetSpriteAnimationRequestOneFrameComponent& request =
				accessor.Get<const pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
			if (request.m_SetRow)
			{
				animation.m_Row = request.m_Row;
			}
			animation.m_Playing = request.m_Playing;
		}

		const int frameCount = std::max(1, animation.m_FrameCount);
		if (animation.m_Playing && animation.m_FrameDuration > 0.f)
		{
			animation.m_Elapsed += deltaSeconds;
			while (animation.m_Elapsed >= animation.m_FrameDuration)
			{
				animation.m_Elapsed -= animation.m_FrameDuration;
				animation.m_Column = (animation.m_Column + 1) % frameCount;
			}
		}
		else if (!animation.m_Playing)
		{
			// Paused: rest on the idle frame so a stopped character shows a clean standing pose.
			animation.m_Column = 0;
			animation.m_Elapsed = 0.f;
		}

		sprite.m_TexCoordsRect = animation.m_Sheet.GetFrameTexCoords(animation.m_Column, animation.m_Row);
	}
}
