#include "Sandbox/QuadRenderSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawQuadInFrameEvent.h"
#include "Pigeon/Transform/WorldTransformComponent.h"
#include "Sandbox/QuadComponent.h"

pg::SystemAccessDecl sbx::QuadRenderSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::QuadComponent)),
		std::type_index(typeid(pg::WorldTransformComponent)),
	};
	decl.inframeAddSet = {
		std::type_index(typeid(pg::DrawQuadInFrameEvent)),
	};
	return decl;
}

void sbx::QuadRenderSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto view = accessor.View<const sbx::QuadComponent, const pg::WorldTransformComponent>();
	for (pg::ecs::Entity ent : view)
	{
		const sbx::QuadComponent& quad = view.get<const sbx::QuadComponent>(ent);
		const pg::WorldTransformComponent& worldTransform = view.get<const pg::WorldTransformComponent>(ent);

		pg::DrawQuadInFrameEvent event;
		event.m_Transform = worldTransform.m_Matrix;
		event.m_SortKey = worldTransform.m_SortKey;
		event.m_Color = quad.m_Color;
		event.m_TextureID = quad.m_TextureID;
		accessor.EmplaceInframeEvent<pg::DrawQuadInFrameEvent>(std::move(event));
	}
}
