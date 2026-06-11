#include "pch.h"
#include "Pigeon/Renderer/SpriteRenderSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Pigeon/Renderer/Sprite.h"
#include "Pigeon/Renderer/SpriteComponent.h"
#include "Pigeon/Transform/WorldTransformComponent.h"

pg::SystemAccessDecl pg::SpriteRenderSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::SpriteComponent)),
		std::type_index(typeid(pg::WorldTransformComponent)),
	};
	decl.inframeAddSet = {
		std::type_index(typeid(pg::DrawSpriteInFrameEvent)),
	};
	return decl;
}

void pg::SpriteRenderSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto view = accessor.View<const pg::SpriteComponent, const pg::WorldTransformComponent>();
	for (pg::ecs::Entity ent : view)
	{
		const pg::SpriteComponent& sprite = view.get<const pg::SpriteComponent>(ent);
		const pg::WorldTransformComponent& worldTransform = view.get<const pg::WorldTransformComponent>(ent);
		pg::Sprite drawnSprite(worldTransform.m_Matrix, sprite.m_TexCoordsRect, sprite.m_TextureID, glm::vec3(0.f));
		accessor.EmplaceInframeEvent<pg::DrawSpriteInFrameEvent>(drawnSprite, worldTransform.m_SortKey);
	}
}
