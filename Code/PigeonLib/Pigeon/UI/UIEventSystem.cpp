#include "UIEventSystem.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

namespace
{
	bool IsPosInsideBounds(const glm::vec2& pos, const glm::vec4& bounds)
	{
		return pos.x >= bounds.x && pos.y >= bounds.y && pos.x <= bounds.x + bounds.z && pos.y <= bounds.y + bounds.w;
	}
}

pg::SystemAccessDecl pg::ui::UIEventSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::InputStateSingletonComponent)),
		std::type_index(typeid(pg::ui::RendererConfigSingletonComponent)),
		std::type_index(typeid(pg::ui::BaseComponent)),
		std::type_index(typeid(pg::ui::UIOnClickOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::UIOnClickOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent)),
	};

	return decl;
}

void pg::ui::UIEventSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto viewInput = accessor.view<const pg::InputStateSingletonComponent>();
	if (viewInput.size() != 1)
		return;

	auto viewRenderConfig = accessor.view<const pg::ui::RendererConfigSingletonComponent>();
	if (viewRenderConfig.size() != 1)
		return;

	const pg::InputStateSingletonComponent& inputComponent = viewInput.get<const pg::InputStateSingletonComponent>(viewInput.front());
	const pg::ui::RendererConfigSingletonComponent& renderComponent = viewRenderConfig.get<const pg::ui::RendererConfigSingletonComponent>(viewRenderConfig.front());

	auto viewUI = accessor.view<const pg::ui::BaseComponent>();
	for (auto ent : viewUI)
	{
		const pg::ui::BaseComponent& baseComponent = viewUI.get<pg::ui::BaseComponent>(ent);
		if (pg::ui::IsUIElementEnabled(accessor, baseComponent))
		{
			int level = 0;
			const glm::vec4 uiBounds = pg::ui::GetGlobalBoundsForElement(accessor, baseComponent, renderComponent, baseComponent.m_Size, level);

			if (IsPosInsideBounds(inputComponent.m_MousePos, uiBounds))
			{
				pg::ui::UIOnHoverOneFrameComponent hoverComp;
				hoverComp.m_ElementID = baseComponent.m_UUID;
				accessor.emplace_oneframe<pg::ui::UIOnHoverOneFrameComponent>(ent, std::move(hoverComp));

				if (inputComponent.m_KeysPressed.find(PG_MOUSE_BUTTON_LEFT) != inputComponent.m_KeysPressed.end())
				{
					pg::ui::UIOnClickOneFrameComponent clickComp;
					clickComp.m_ElementID = baseComponent.m_UUID;
					accessor.emplace_oneframe<pg::ui::UIOnClickOneFrameComponent>(ent, std::move(clickComp));
				}
				else if (inputComponent.m_KeysReleased.find(PG_MOUSE_BUTTON_LEFT) != inputComponent.m_KeysReleased.end())
				{
					pg::ui::UIOnReleaseOneFrameComponent releaseComp;
					releaseComp.m_ElementID = baseComponent.m_UUID;
					accessor.emplace_oneframe<pg::ui::UIOnReleaseOneFrameComponent>(ent, std::move(releaseComp));
				}
			}
		}
	}
}
