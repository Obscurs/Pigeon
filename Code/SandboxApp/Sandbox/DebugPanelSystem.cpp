#include "Sandbox/DebugPanelSystem.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/DebugControlsSingletonComponent.h"
#include "Sandbox/SpawnerSingletonComponent.h"

#include <imgui.h>

pg::SystemAccessDecl sbx::DebugPanelSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SpawnerSingletonComponent)),
		std::type_index(typeid(pg::InputStateSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::DebugControlsSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::DebugControlsSingletonComponent)),
	};
	return decl;
}

void sbx::DebugPanelSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto controlsView = accessor.View<sbx::DebugControlsSingletonComponent>();
	if (controlsView.empty())
	{
		pg::ecs::Entity ent = accessor.Create();
		accessor.EmplaceDeferred<sbx::DebugControlsSingletonComponent>(ent);
		return;
	}

	// The test build pushes no ImGuiLayer, so no ImGui context exists there; guard every call.
	if (ImGui::GetCurrentContext() == nullptr)
	{
		return;
	}

	sbx::DebugControlsSingletonComponent& controls = controlsView.get<sbx::DebugControlsSingletonComponent>(controlsView.front());

	ImGui::Begin("Pigeon Showcase");
	ImGui::Text("Engine features driven entirely through ECS systems.");
	ImGui::SliderFloat("Animation speed", &controls.m_AnimationSpeed, 0.f, 3.f);

	auto spawnerView = accessor.View<const sbx::SpawnerSingletonComponent>();
	if (!spawnerView.empty())
	{
		ImGui::Text("Spawned quads: %d", spawnerView.get<const sbx::SpawnerSingletonComponent>(spawnerView.front()).m_SpawnCount);
	}

	auto inputView = accessor.View<const pg::InputStateSingletonComponent>();
	if (!inputView.empty())
	{
		const pg::InputStateSingletonComponent& input = inputView.get<const pg::InputStateSingletonComponent>(inputView.front());
		ImGui::Text("Mouse: %.0f, %.0f", input.m_MousePos.x, input.m_MousePos.y);
		ImGui::Text("Keys held: %d", static_cast<int>(input.m_KeysPressed.size()));
	}

	ImGui::End();
}
