#include "Sandbox/UIButtonSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UIElementHelpers.h"

pg::SystemAccessDecl sbx::UIButtonSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::UIUpdateEnableOneFrameComponent)),
	};
	return decl;
}

void sbx::UIButtonSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	if (configView.empty())
	{
		return;
	}
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	if (!sbx::IsElementReleased(accessor, config.m_ToggleButtonID))
	{
		return;
	}

	pg::ecs::Entity panel = sbx::FindUIElementByUUID(accessor, config.m_TogglePanelID);
	if (panel == pg::ecs::null)
	{
		return;
	}

	const pg::ui::BaseComponent& panelBase = accessor.Get<const pg::ui::BaseComponent>(panel);
	pg::ui::UIUpdateEnableOneFrameComponent update;
	update.m_Enabled = !panelBase.m_Enabled;
	accessor.EmplaceOneframe<pg::ui::UIUpdateEnableOneFrameComponent>(panel, std::move(update));
}
