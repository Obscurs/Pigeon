#include "Sandbox/TransformResolveSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/ResolvedTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/TransformRequestData.h"
#include "Sandbox/AnimationTransformRequestOneFrameComponent.h"
#include "Sandbox/QuadSpawnTransformRequestOneFrameComponent.h"
#include "Sandbox/SceneTransformRequestOneFrameComponent.h"

#include <utility>
#include <vector>

namespace
{
	struct EntityRequest
	{
		pg::ecs::Entity m_Entity;
		pg::TransformRequestData m_Data;
	};

	void MergeChannels(pg::TransformRequestData& out, const pg::TransformRequestData& in)
	{
		if (in.m_SetPosition)
		{
			out.m_SetPosition = true;
			out.m_Position = in.m_Position;
		}
		if (in.m_SetRotation)
		{
			out.m_SetRotation = true;
			out.m_Rotation = in.m_Rotation;
		}
		if (in.m_SetScale)
		{
			out.m_SetScale = true;
			out.m_Scale = in.m_Scale;
		}
		if (in.m_SetBounds)
		{
			out.m_SetBounds = true;
			out.m_BoundsMin = in.m_BoundsMin;
			out.m_BoundsMax = in.m_BoundsMax;
		}
	}

	pg::TransformRequestData& FindOrAdd(std::vector<EntityRequest>& list, pg::ecs::Entity ent)
	{
		for (EntityRequest& request : list)
		{
			if (request.m_Entity == ent)
			{
				return request.m_Data;
			}
		}
		list.push_back({ ent, pg::TransformRequestData{} });
		return list.back().m_Data;
	}

	template<typename RequestComponent>
	void Gather(pg::CheckedRegistryAccessor& accessor, std::vector<EntityRequest>& list)
	{
		auto view = accessor.View<const RequestComponent>();
		for (pg::ecs::Entity ent : view)
		{
			MergeChannels(FindOrAdd(list, ent), view.get<const RequestComponent>(ent).m_Data);
		}
	}
}

pg::SystemAccessDecl sbx::TransformResolveSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::SceneTransformRequestOneFrameComponent)),
		std::type_index(typeid(sbx::QuadSpawnTransformRequestOneFrameComponent)),
		std::type_index(typeid(sbx::AnimationTransformRequestOneFrameComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ResolvedTransformRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::TransformResolveSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	std::vector<EntityRequest> requests;
	Gather<sbx::SceneTransformRequestOneFrameComponent>(accessor, requests);
	Gather<sbx::QuadSpawnTransformRequestOneFrameComponent>(accessor, requests);
	Gather<sbx::AnimationTransformRequestOneFrameComponent>(accessor, requests);

	for (const EntityRequest& request : requests)
	{
		pg::ResolvedTransformRequestOneFrameComponent resolved;
		resolved.m_Data = request.m_Data;
		accessor.EmplaceOneframe<pg::ResolvedTransformRequestOneFrameComponent>(request.m_Entity, std::move(resolved));
	}
}
