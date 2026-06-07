#include "Sandbox/InputReadoutSystem.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/InputReadoutTagComponent.h"
#include "Sandbox/LabelComponent.h"

namespace
{
	std::string FormatInput(const pg::InputStateSingletonComponent& input)
	{
		const int mouseX = static_cast<int>(input.m_MousePos.x);
		const int mouseY = static_cast<int>(input.m_MousePos.y);
		const int scrollX = static_cast<int>(input.m_MouseScroll.x);
		const int scrollY = static_cast<int>(input.m_MouseScroll.y);
		return "Mouse " + std::to_string(mouseX) + "," + std::to_string(mouseY)
			+ "   Held keys " + std::to_string(input.m_KeysPressed.size())
			+ "   Scroll " + std::to_string(scrollX) + "," + std::to_string(scrollY);
	}
}

pg::SystemAccessDecl sbx::InputReadoutSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::InputStateSingletonComponent)),
		std::type_index(typeid(sbx::InputReadoutTagComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::LabelComponent)),
	};
	return decl;
}

void sbx::InputReadoutSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto inputView = accessor.View<const pg::InputStateSingletonComponent>();
	if (inputView.empty())
	{
		return;
	}
	const pg::InputStateSingletonComponent& input = inputView.get<const pg::InputStateSingletonComponent>(inputView.front());
	const std::string text = FormatInput(input);

	auto view = accessor.View<const sbx::InputReadoutTagComponent, sbx::LabelComponent>();
	for (pg::ecs::Entity ent : view)
	{
		view.get<sbx::LabelComponent>(ent).m_Text = text;
	}
}
