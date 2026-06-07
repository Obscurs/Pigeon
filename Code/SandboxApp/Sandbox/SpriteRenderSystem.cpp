#include "Sandbox/SpriteRenderSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Pigeon/Renderer/Sprite.h"
#include "Sandbox/SpriteComponent.h"

pg::SystemAccessDecl sbx::SpriteRenderSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SpriteComponent)),
	};
	decl.inframeAddSet = {
		std::type_index(typeid(pg::DrawSpriteInFrameEvent)),
	};
	return decl;
}

void sbx::SpriteRenderSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto view = accessor.View<const sbx::SpriteComponent>();
	for (pg::ecs::Entity ent : view)
	{
		const sbx::SpriteComponent& sprite = view.get<const sbx::SpriteComponent>(ent);
		pg::Sprite drawnSprite(sprite.m_Transform, sprite.m_TexCoordsRect, sprite.m_TextureID, sprite.m_Origin);
		accessor.EmplaceInframeEvent<pg::DrawSpriteInFrameEvent>(drawnSprite);
	}
}
