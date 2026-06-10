#include "Pigeon/UI/UIEventSystem.h"

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
		std::type_index(typeid(pg::ui::LayoutContainerComponent)),
		std::type_index(typeid(pg::ui::UIClipComponent)),
		std::type_index(typeid(pg::ui::UIOnClickOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ui::UIOnClickOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnHoverOneFrameComponent)),
		std::type_index(typeid(pg::ui::UIOnReleaseOneFrameComponent)),
	};
	decl.inframeAddSet = {
		std::type_index(typeid(pg::ui::UIInputCapturedInFrameEvent)),
	};

	return decl;
}

void pg::ui::UIEventSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto viewInput = accessor.View<const pg::InputStateSingletonComponent>();
	if (viewInput.size() != 1)
		return;

	auto viewRenderConfig = accessor.View<const pg::ui::RendererConfigSingletonComponent>();
	if (viewRenderConfig.size() != 1)
		return;

	const pg::InputStateSingletonComponent& inputComponent = viewInput.get<const pg::InputStateSingletonComponent>(viewInput.front());
	const pg::ui::RendererConfigSingletonComponent& renderComponent = viewRenderConfig.get<const pg::ui::RendererConfigSingletonComponent>(viewRenderConfig.front());

	// Bounds resolve in logical canvas units; the mouse arrives in window pixels. Convert it into canvas
	// units (the inverse of the UI scale factor) so hit-testing matches rendering at any resolution.
	const float scaleFactor = renderComponent.m_ScaleFactor > 0.f ? renderComponent.m_ScaleFactor : 1.f;
	const glm::vec2 canvasMousePos = inputComponent.m_MousePos / scaleFactor;

	// Resolve the single front-most element under the cursor: greatest nesting depth wins, and a later
	// iteration wins ties (matching the draw order where deeper / later elements render on top). Top-most
	// input capture means events route to that element only, never leaking to elements behind it.
	pg::ecs::Entity frontEntity = pg::ecs::null;
	int frontDepth = -1;
	pg::UUID frontUUID;

	auto viewUI = accessor.View<const pg::ui::BaseComponent>();
	for (auto ent : viewUI)
	{
		const pg::ui::BaseComponent& baseComponent = viewUI.get<const pg::ui::BaseComponent>(ent);
		if (!pg::ui::IsUIElementEnabled(accessor, baseComponent))
			continue;

		const glm::vec4 uiBounds = pg::ui::GetElementRect(accessor, baseComponent, renderComponent);
		if (!IsPosInsideBounds(canvasMousePos, uiBounds))
			continue;

		const int depth = pg::ui::GetElementDepth(accessor, baseComponent);
		if (depth >= frontDepth)
		{
			frontDepth = depth;
			frontEntity = ent;
			frontUUID = baseComponent.m_UUID;
		}
	}

	if (frontEntity == pg::ecs::null)
		return;

	pg::ui::UIOnHoverOneFrameComponent hoverComp;
	hoverComp.m_ElementID = frontUUID;
	accessor.EmplaceOneframe<pg::ui::UIOnHoverOneFrameComponent>(frontEntity, std::move(hoverComp));

	if (inputComponent.m_KeysPressed.find(PG_MOUSE_BUTTON_LEFT) != inputComponent.m_KeysPressed.end())
	{
		pg::ui::UIOnClickOneFrameComponent clickComp;
		clickComp.m_ElementID = frontUUID;
		accessor.EmplaceOneframe<pg::ui::UIOnClickOneFrameComponent>(frontEntity, std::move(clickComp));
	}
	else if (inputComponent.m_KeysReleased.find(PG_MOUSE_BUTTON_LEFT) != inputComponent.m_KeysReleased.end())
	{
		pg::ui::UIOnReleaseOneFrameComponent releaseComp;
		releaseComp.m_ElementID = frontUUID;
		accessor.EmplaceOneframe<pg::ui::UIOnReleaseOneFrameComponent>(frontEntity, std::move(releaseComp));
	}

	// Raise the capture signal only for interactive elements (those with an image or text), so bare
	// layout containers (e.g. a full-canvas root) do not block gameplay pointer input over empty areas.
	if (accessor.AnyOf<pg::ui::ImageComponent>(frontEntity) || accessor.AnyOf<pg::ui::TextComponent>(frontEntity))
	{
		pg::ui::UIInputCapturedInFrameEvent capturedEvent;
		capturedEvent.m_ElementID = frontUUID;
		accessor.EmplaceInframeEvent<pg::ui::UIInputCapturedInFrameEvent>(std::move(capturedEvent));
	}
}
