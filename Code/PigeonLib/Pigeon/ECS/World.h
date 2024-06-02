#pragma once
#include <entt/entt.hpp>

#include "Pigeon/ECS/System.h"

namespace pig
{
	class World
	{
	public:
		World() = default;
		static World& Create()
		{
			s_Instance = std::make_unique<World>();
			s_Instance->Init();
			return s_Instance->Get();
		}

		void Update(float dt);
		void RegisterSystem(std::unique_ptr<pig::System> system);

		inline static World& Get() { return *s_Instance; }
		inline static entt::registry& GetRegistry() { return s_Instance->m_Registry; }

	private:
		void Init();

		entt::registry m_Registry;
		std::vector<pig::U_Ptr<pig::System>> m_Systems;

		static pig::U_Ptr<World> s_Instance;
	};
}
