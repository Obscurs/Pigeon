#include "Pigeon/Transform/TransformResolveSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/CameraTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/LocalBoundsComponent.h"
#include "Pigeon/Transform/PositionComponent.h"
#include "Pigeon/Transform/ResolvedTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/RotationComponent.h"
#include "Pigeon/Transform/ScaleComponent.h"
#include "Pigeon/Transform/TransformRequestData.h"

#include <utility>

namespace
{
	void SetPosition(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent, const glm::vec3& value)
	{
		if (accessor.AllOf<pg::PositionComponent>(ent))
		{
			accessor.Get<pg::PositionComponent>(ent).m_Position = value;
		}
		else
		{
			pg::PositionComponent comp;
			comp.m_Position = value;
			accessor.EmplaceDeferred<pg::PositionComponent>(ent, std::move(comp));
		}
	}

	void SetRotation(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent, const glm::quat& value)
	{
		if (accessor.AllOf<pg::RotationComponent>(ent))
		{
			accessor.Get<pg::RotationComponent>(ent).m_Rotation = value;
		}
		else
		{
			pg::RotationComponent comp;
			comp.m_Rotation = value;
			accessor.EmplaceDeferred<pg::RotationComponent>(ent, std::move(comp));
		}
	}

	void SetScale(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent, const glm::vec3& value)
	{
		if (accessor.AllOf<pg::ScaleComponent>(ent))
		{
			accessor.Get<pg::ScaleComponent>(ent).m_Scale = value;
		}
		else
		{
			pg::ScaleComponent comp;
			comp.m_Scale = value;
			accessor.EmplaceDeferred<pg::ScaleComponent>(ent, std::move(comp));
		}
	}

	void SetBounds(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent, const glm::vec3& min, const glm::vec3& max)
	{
		if (accessor.AllOf<pg::LocalBoundsComponent>(ent))
		{
			pg::LocalBoundsComponent& bounds = accessor.Get<pg::LocalBoundsComponent>(ent);
			bounds.m_Min = min;
			bounds.m_Max = max;
		}
		else
		{
			pg::LocalBoundsComponent comp;
			comp.m_Min = min;
			comp.m_Max = max;
			accessor.EmplaceDeferred<pg::LocalBoundsComponent>(ent, std::move(comp));
		}
	}

	void ApplyRequest(pg::CheckedRegistryAccessor& accessor, pg::ecs::Entity ent, const pg::TransformRequestData& data)
	{
		if (data.m_SetPosition)
		{
			SetPosition(accessor, ent, data.m_Position);
		}
		if (data.m_SetRotation)
		{
			SetRotation(accessor, ent, data.m_Rotation);
		}
		if (data.m_SetScale)
		{
			SetScale(accessor, ent, data.m_Scale);
		}
		if (data.m_SetBounds)
		{
			SetBounds(accessor, ent, data.m_BoundsMin, data.m_BoundsMax);
		}
	}
}

pg::SystemAccessDecl pg::TransformResolveSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::ResolvedTransformRequestOneFrameComponent)),
		std::type_index(typeid(pg::CameraTransformRequestOneFrameComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::PositionComponent)),
		std::type_index(typeid(pg::RotationComponent)),
		std::type_index(typeid(pg::ScaleComponent)),
		std::type_index(typeid(pg::LocalBoundsComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::PositionComponent)),
		std::type_index(typeid(pg::RotationComponent)),
		std::type_index(typeid(pg::ScaleComponent)),
		std::type_index(typeid(pg::LocalBoundsComponent)),
	};
	return decl;
}

void pg::TransformResolveSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto resolvedView = accessor.View<const pg::ResolvedTransformRequestOneFrameComponent>();
	for (pg::ecs::Entity ent : resolvedView)
	{
		ApplyRequest(accessor, ent, resolvedView.get<const pg::ResolvedTransformRequestOneFrameComponent>(ent).m_Data);
	}

	auto cameraView = accessor.View<const pg::CameraTransformRequestOneFrameComponent>();
	for (pg::ecs::Entity ent : cameraView)
	{
		ApplyRequest(accessor, ent, cameraView.get<const pg::CameraTransformRequestOneFrameComponent>(ent).m_Data);
	}
}
