#include "Sandbox/UIButtonVisualSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/UIButtonVisualStateSingletonComponent.h"
#include "Sandbox/UIElementHelpers.h"

pg::SystemAccessDecl sbx::UIButtonVisualSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
		std::type_index(typeid(pg::ui::UIOnClickOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::UIButtonVisualStateSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::UIUpdateImageUUIDOneFrameComponent)),
		std::type_index(typeid(sbx::UIButtonVisualStateSingletonComponent)),
	};
	return decl;
}

void sbx::UIButtonVisualSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto configView = accessor.View<const sbx::SandboxConfigSingletonComponent>();
	if (configView.empty())
	{
		return;
	}
	const sbx::SandboxConfigSingletonComponent& config = configView.get<const sbx::SandboxConfigSingletonComponent>(configView.front());

	auto stateView = accessor.View<sbx::UIButtonVisualStateSingletonComponent>();
	if (stateView.empty())
	{
		pg::ecs::Entity stateEnt = accessor.Create();
		accessor.EmplaceDeferred<sbx::UIButtonVisualStateSingletonComponent>(stateEnt);
		return;
	}

	pg::ecs::Entity button = sbx::FindUIElementByUUID(accessor, config.m_ToggleButtonID);
	if (button == pg::ecs::null)
	{
		return;
	}

	const int desiredState = sbx::IsElementClicked(accessor, config.m_ToggleButtonID) ? 2
		: (sbx::IsElementHovered(accessor, config.m_ToggleButtonID) ? 1 : 0);

	sbx::UIButtonVisualStateSingletonComponent& state = stateView.get<sbx::UIButtonVisualStateSingletonComponent>(stateView.front());
	if (desiredState == state.m_State)
	{
		return;
	}
	state.m_State = desiredState;

	pg::ui::UIUpdateImageUUIDOneFrameComponent update;
	update.m_UUID = desiredState == 2 ? config.m_ButtonPressedImageID
		: (desiredState == 1 ? config.m_ButtonHoverImageID : config.m_ButtonImageID);
	accessor.EmplaceOneframe<pg::ui::UIUpdateImageUUIDOneFrameComponent>(button, std::move(update));
}
