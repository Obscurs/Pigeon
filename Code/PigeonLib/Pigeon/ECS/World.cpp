#include "pch.h"

#include "World.h"

#include "Pigeon/ECS/System.h"

pig::U_Ptr<pig::World> pig::World::s_Instance = nullptr;

void pig::World::Update(const pig::Timestep& ts)
{
	for (int i=0; i < m_Systems.size(); ++i)
	{
		m_Systems[i]->Update(ts);
	}
}

void pig::World::RegisterSystem(std::unique_ptr<pig::System> system)
{
	const std::type_index typeIdx = std::type_index(typeid(*system.get()));
	PG_CORE_EXCEPT(m_SystemTypes.find(typeIdx) == m_SystemTypes.end(), "System already registered");
	
	m_SystemTypes.insert(typeIdx);
	m_Systems.push_back(std::move(system));
}

void pig::World::Init()
{
	m_Systems.clear();
	m_Registry.clear();
}

