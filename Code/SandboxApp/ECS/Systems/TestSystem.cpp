#include "pch.h"

#include "ECS/Systems/TestSystem.h"
#include "ECS/Components/TestComponent.h"

#include "Pigeon/ECS/World.h"

sbx::TestSystem::TestSystem()
{
	// Entity seeding happens outside of a system Update() — use GetRegistryDirect().
	// Uncomment to seed initial test entities:
	/*entt::registry& reg = pig::World::GetRegistryDirect();
	entt::entity testSingletonEntity = reg.create();
	reg.emplace<sbx::TestComponent>(testSingletonEntity, 1, 2);*/
}

pig::SystemAccessDecl sbx::TestSystem::DeclareAccess() const
{
	// TestSystem is a read-only observer. The commented-out write code must stay commented out.
	// If write logic is ever re-enabled, add TestComponent to writeSet at that time.
	pig::SystemAccessDecl decl;
	decl.readSet = { std::type_index(typeid(sbx::TestComponent)) };
	return decl;
}

void sbx::TestSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();
	auto view = accessor.view<sbx::TestComponent>();
	for (auto ent : view)
	{
		sbx::TestComponent& tc = view.get<sbx::TestComponent>(ent);
		//tc.m_X += 1;
		//tc.m_Y += tc.m_X * 2;
		//std::cout << tc.m_X << " " << tc.m_Y << std::endl;
	}
}
