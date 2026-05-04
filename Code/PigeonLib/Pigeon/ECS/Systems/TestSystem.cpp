#include "TestSystem.h"
//ARNAU TODO, rething placement of these files
#include "Pigeon/ECS/Components/TestComponent.h"

#include "Pigeon/ECS/World.h"

pig::TestSystem::TestSystem()
{
	//Uncomment this code to test
	/*entt::entity testSingletonEntity = pig::World::GetRegistry().create();

	pig::World::GetRegistry().emplace<pig::TestComponent>(testSingletonEntity, 1, 2);*/
}

void pig::TestSystem::Update(const pig::Timestep& ts)
{
	auto view = pig::World::GetRegistry().view<pig::TestComponent>();
	for (auto ent : view)
	{
		TestComponent& tc = view.get<pig::TestComponent>(ent);
		//tc.m_X += 1;
		//tc.m_Y += tc.m_X * 2;
		//std::cout << tc.m_X << " " << tc.m_Y << std::endl;
	}
}
