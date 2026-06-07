#include "Sandbox/UICloseSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UIElementHelpers.h"

pg::SystemAccessDecl sbx::UICloseSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::UIDestroyOneFrameComponent)),
	};
	return decl;
}

void sbx::UICloseSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	if (configView.empty())
	{
		return;
	}
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	if (!sbx::IsElementReleased(accessor, config.m_CloseButtonID))
	{
		return;
	}

	pg::ecs::Entity target = sbx::FindUIElementByUUID(accessor, config.m_CloseTargetID);
	if (target == pg::ecs::null)
	{
		return;
	}

	accessor.EmplaceOneframe<pg::ui::UIDestroyOneFrameComponent>(target);
}
