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

	void CleanOneFrameComponents(pig::CheckedRegistryAccessor& accessor)
	{
		auto viewClick = accessor.view<const pig::ui::UIOnClickOneFrameComponent>();
		for (auto ent : viewClick)
		{
			accessor.remove_deferred<pig::ui::UIOnClickOneFrameComponent>(ent);
		}
		auto viewHover = accessor.view<const pig::ui::UIOnHoverOneFrameComponent>();
		for (auto ent : viewHover)
		{
			accessor.remove_deferred<pig::ui::UIOnHoverOneFrameComponent>(ent);
		}
		auto viewRelease = accessor.view<const pig::ui::UIOnReleaseOneFrameComponent>();
		for (auto ent : viewRelease)
		{
			accessor.remove_deferred<pig::ui::UIOnReleaseOneFrameComponent>(ent);
		}
	}
}

pig::SystemAccessDecl pig::ui::UIEventSystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pig::InputStateSingletonComponent)),
		std::type_index(typeid(pig::ui::RendererConfig)),
		std::type_index(typeid(pig::ui::BaseComponent)),
		std::type_index(typeid(pig::ui::UIOnClickOneFrameComponent)),
		std::type_index(typeid(pig::ui::UIOnHoverOneFrameComponent)),
		std::type_index(typeid(pig::ui::UIOnReleaseOneFrameComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pig::ui::UIOnClickOneFrameComponent)),
		std::type_index(typeid(pig::ui::UIOnHoverOneFrameComponent)),
		std::type_index(typeid(pig::ui::UIOnReleaseOneFrameComponent)),
	};

	return decl;
}

void pig::ui::UIEventSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	CleanOneFrameComponents(accessor);

	auto viewInput = accessor.view<const pig::InputStateSingletonComponent>();
	if (viewInput.size() != 1)
		return;

	auto viewRenderConfig = accessor.view<const pig::ui::RendererConfig>();
	if (viewRenderConfig.size() != 1)
		return;

	const pig::InputStateSingletonComponent& inputComponent = viewInput.get<const pig::InputStateSingletonComponent>(viewInput.front());
	const pig::ui::RendererConfig& renderComponent = viewRenderConfig.get<const pig::ui::RendererConfig>(viewRenderConfig.front());

	auto viewUI = accessor.view<const pig::ui::BaseComponent>();
	for (auto ent : viewUI)
	{
		const pig::ui::BaseComponent& baseComponent = viewUI.get<pig::ui::BaseComponent>(ent);
		if (pig::ui::IsUIElementEnabled(accessor, baseComponent))
		{
			int level = 0;
			const glm::vec4 uiBounds = pig::ui::GetGlobalBoundsForElement(accessor, baseComponent, renderComponent, baseComponent.m_Size, level);

			if (IsPosInsideBounds(inputComponent.m_MousePos, uiBounds))
			{
				pig::ui::UIOnHoverOneFrameComponent hoverComp;
				hoverComp.m_ElementID = baseComponent.m_UUID;
				accessor.emplace_deferred<pig::ui::UIOnHoverOneFrameComponent>(ent, std::move(hoverComp));

				if (inputComponent.m_KeysPressed.find(PG_MOUSE_BUTTON_LEFT) != inputComponent.m_KeysPressed.end())
				{
					pig::ui::UIOnClickOneFrameComponent clickComp;
					clickComp.m_ElementID = baseComponent.m_UUID;
					accessor.emplace_deferred<pig::ui::UIOnClickOneFrameComponent>(ent, std::move(clickComp));
				}
				else if (inputComponent.m_KeysReleased.find(PG_MOUSE_BUTTON_LEFT) != inputComponent.m_KeysReleased.end())
				{
					pig::ui::UIOnReleaseOneFrameComponent releaseComp;
					releaseComp.m_ElementID = baseComponent.m_UUID;
					accessor.emplace_deferred<pig::ui::UIOnReleaseOneFrameComponent>(ent, std::move(releaseComp));
				}
			}
		}
	}
}
