#include "Sandbox/UIStatusSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/SpawnerSingletonComponent.h"
#include "Sandbox/StatusDisplaySingletonComponent.h"
#include "Sandbox/UIElementHelpers.h"

pg::SystemAccessDecl sbx::UIStatusSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SpawnerSingletonComponent)),
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::StatusDisplaySingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::UIUpdateTextOneFrameComponent)),
		std::type_index(typeid(sbx::StatusDisplaySingletonComponent)),
	};
	return decl;
}

void sbx::UIStatusSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto spawnerView = accessor.View<const sbx::SpawnerSingletonComponent>();
	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	if (spawnerView.empty() || configView.empty())
	{
		return;
	}
	const sbx::SpawnerSingletonComponent& spawner = spawnerView.get<const sbx::SpawnerSingletonComponent>(spawnerView.front());
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	auto displayView = accessor.View<sbx::StatusDisplaySingletonComponent>();
	if (displayView.empty())
	{
		pg::ecs::Entity displayEnt = accessor.Create();
		accessor.EmplaceDeferred<sbx::StatusDisplaySingletonComponent>(displayEnt);
		return;
	}
	sbx::StatusDisplaySingletonComponent& display = displayView.get<sbx::StatusDisplaySingletonComponent>(displayView.front());
	if (spawner.m_SpawnCount == display.m_LastSpawnCount)
	{
		return;
	}

	pg::ecs::Entity statusEnt = sbx::FindUIElementByUUID(accessor, config.m_StatusTextID);
	if (statusEnt == pg::ecs::null)
	{
		return;
	}
	display.m_LastSpawnCount = spawner.m_SpawnCount;

	pg::ui::UIUpdateTextOneFrameComponent update;
	update.m_Text = "Spawned: " + std::to_string(spawner.m_SpawnCount);
	update.m_FontID = config.m_DefaultFontID;
	update.m_Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	update.m_Kerning = 0.1f;
	update.m_Spacing = 0.1f;
	accessor.EmplaceOneframe<pg::ui::UIUpdateTextOneFrameComponent>(statusEnt, std::move(update));
}
