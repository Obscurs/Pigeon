#include "pch.h"

#include "World.h"

#include "Pigeon/ECS/System.h"

pig::U_Ptr<pig::World> pig::World::s_Instance = nullptr;

void pig::World::Update(float dt)
{
	for (int i=0; i < m_Systems.size(); ++i)
	{
		m_Systems[i]->Update(dt);
	}
}

void pig::World::RegisterSystem(std::unique_ptr<pig::System> system)
{
	PG_CORE_ASSERT(system->IsValid(), "Trying to push an invalid system");
	for (const auto& sys : m_Systems)
	{
		PG_CORE_ASSERT(system->GetType() != sys->GetType(), "System type already registered");
	}
	m_Systems.push_back(std::move(system));
}

void pig::World::Init()
{
	m_Systems.clear();
	m_Registry.clear();
}

