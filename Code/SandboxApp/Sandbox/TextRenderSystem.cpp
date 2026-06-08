#include "Sandbox/TextRenderSystem.h"

#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawStringInFrameEvent.h"
#include "Pigeon/Transform/WorldTransformComponent.h"
#include "Sandbox/LabelComponent.h"

pg::SystemAccessDecl sbx::TextRenderSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(sbx::LabelComponent)),
		std::type_index(typeid(pg::WorldTransformComponent)),
	};
	decl.inframeAddSet = {
		std::type_index(typeid(pg::DrawStringInFrameEvent)),
	};
	return decl;
}

void sbx::TextRenderSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto resourcesView = accessor.View<const pg::ResourceMapSingletonComponent>();
	if (resourcesView.empty())
	{
		return;
	}
	const pg::ResourceMapSingletonComponent& resources = resourcesView.get<const pg::ResourceMapSingletonComponent>(resourcesView.front());

	auto view = accessor.View<const sbx::LabelComponent, const pg::WorldTransformComponent>();
	for (pg::ecs::Entity ent : view)
	{
		const sbx::LabelComponent& label = view.get<const sbx::LabelComponent>(ent);
		const pg::WorldTransformComponent& worldTransform = view.get<const pg::WorldTransformComponent>(ent);
		const std::unordered_map<pg::UUID, pg::S_Ptr<pg::Font>>::const_iterator fontIt = resources.m_FontMap.find(label.m_FontID);
		if (fontIt == resources.m_FontMap.end())
		{
			continue;
		}

		pg::DrawStringInFrameEvent event;
		event.m_Transform = worldTransform.m_Matrix;
		event.m_SortKey = worldTransform.m_SortKey;
		event.m_String = label.m_Text;
		event.m_Font = fontIt->second;
		event.m_Color = label.m_Color;
		event.m_Kerning = label.m_Kerning;
		event.m_Linespacing = label.m_Linespacing;
		accessor.EmplaceInframeEvent<pg::DrawStringInFrameEvent>(std::move(event));
	}
}
