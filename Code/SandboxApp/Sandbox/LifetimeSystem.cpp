#include "Sandbox/LifetimeSystem.h"

#include "Pigeon/ECS/World.h"
#include "Sandbox/LifetimeComponent.h"

pg::SystemAccessDecl sbx::LifetimeSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.writeSet = {
		std::type_index(typeid(sbx::LifetimeComponent)),
	};
	return decl;
}

void sbx::LifetimeSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	const float deltaSeconds = ts.AsSeconds();

	auto view = accessor.View<sbx::LifetimeComponent>();
	for (pg::ecs::Entity ent : view)
	{
		sbx::LifetimeComponent& lifetime = view.get<sbx::LifetimeComponent>(ent);
		lifetime.m_Remaining -= deltaSeconds;
		if (lifetime.m_Remaining <= 0.f)
		{
			accessor.DestroyDeferred(ent);
		}
	}
}
