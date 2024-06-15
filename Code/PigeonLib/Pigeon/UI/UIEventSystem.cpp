#include "UIEventSystem.h"

#include <Pigeon/Core/InputComponents.h>
#include <Pigeon/Core/KeyCodes.h>

#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/UI/UIHelpers.h"

#include "Pigeon/ECS/World.h"

namespace
{
	bool IsPosInsideBounds(const glm::vec2& pos, const glm::vec4& bounds)
	{
		return pos.x >= bounds.x && pos.y >= bounds.y && pos.x <= bounds.x + bounds.z && pos.y <= bounds.y + bounds.w;
	}
}
pig::ui::UIEventSystem::UIEventSystem():
	pig::System(pig::SystemType::eTest)
{
}

void pig::ui::UIEventSystem::Update(float dt)
{
	CleanOneFrameComponents();

	auto viewInput = pig::World::GetRegistry().view<pig::InputStateSingletonComponent>();
	if (viewInput.size() != 1)
		return;

	auto viewRenderConfig = pig::World::GetRegistry().view<const pig::ui::RendererConfig>();
	if (viewRenderConfig.size() != 1)
		return;

	const pig::InputStateSingletonComponent& inputComponent = viewInput.get<const pig::InputStateSingletonComponent>(viewInput.front());
	const pig::ui::RendererConfig& renderComponent = viewRenderConfig.get<const pig::ui::RendererConfig>(viewRenderConfig.front());

	auto viewUI = pig::World::GetRegistry().view<const pig::ui::BaseComponent>();
	for (auto ent : viewUI)
	{
		const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
		if (pig::ui::IsUIElementEnabled(baseComponent))
		{
			int level = 0;
			const glm::vec4 uiBounds = pig::ui::GetGlobalBoundsForElement(baseComponent, renderComponent, baseComponent.m_Size, level);

			if (IsPosInsideBounds(inputComponent.m_MousePos, uiBounds))
			{
				pig::ui::UIOnHoverOneFrameComponent& hoverComponent = pig::World::GetRegistry().emplace<pig::ui::UIOnHoverOneFrameComponent>(ent);
				hoverComponent.m_ElementID = baseComponent.m_UUID;

				if (inputComponent.m_KeysPressed.find(PG_MOUSE_BUTTON_LEFT) != inputComponent.m_KeysPressed.end())
				{
					pig::ui::UIOnClickOneFrameComponent& clickComponent = pig::World::GetRegistry().emplace<pig::ui::UIOnClickOneFrameComponent>(ent);
					clickComponent.m_ElementID = baseComponent.m_UUID;
				}
				else if (inputComponent.m_KeysReleased.find(PG_MOUSE_BUTTON_LEFT) != inputComponent.m_KeysReleased.end())
				{
					pig::ui::UIOnReleaseOneFrameComponent& releaseComponent = pig::World::GetRegistry().emplace<pig::ui::UIOnReleaseOneFrameComponent>(ent);
					releaseComponent.m_ElementID = baseComponent.m_UUID;
				}
			}
		}
	}
}

void pig::ui::UIEventSystem::CleanOneFrameComponents()
{
	//ARNAU TODO: automatize one frame components?
	//ARNAU TODO clean, do not get registry on each iter
	auto viewClick = pig::World::GetRegistry().view<const pig::ui::UIOnClickOneFrameComponent>();
	for (auto ent : viewClick)
	{
		pig::World::GetRegistry().remove<pig::ui::UIOnClickOneFrameComponent>(ent);
	}
	auto viewHover = pig::World::GetRegistry().view<const pig::ui::UIOnHoverOneFrameComponent>();
	for (auto ent : viewHover)
	{
		pig::World::GetRegistry().remove<pig::ui::UIOnHoverOneFrameComponent>(ent);
	}
	auto viewRelease = pig::World::GetRegistry().view<const pig::ui::UIOnReleaseOneFrameComponent>();
	for (auto ent : viewRelease)
	{
		pig::World::GetRegistry().remove<pig::ui::UIOnReleaseOneFrameComponent>(ent);
	}
}
