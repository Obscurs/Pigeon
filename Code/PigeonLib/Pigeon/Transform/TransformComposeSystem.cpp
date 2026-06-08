#include "Pigeon/Transform/TransformComposeSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/LocalBoundsComponent.h"
#include "Pigeon/Transform/PositionComponent.h"
#include "Pigeon/Transform/RotationComponent.h"
#include "Pigeon/Transform/ScaleComponent.h"
#include "Pigeon/Transform/WorldTransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <utility>

namespace
{
	glm::mat4 ComposeMatrix(const pg::PositionComponent& position, const pg::RotationComponent& rotation, const pg::ScaleComponent& scale)
	{
		glm::mat4 matrix = glm::translate(glm::mat4(1.f), position.m_Position);
		matrix = matrix * glm::mat4_cast(rotation.m_Rotation);
		matrix = glm::scale(matrix, scale.m_Scale);
		return matrix;
	}
}

pg::SystemAccessDecl pg::TransformComposeSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::PositionComponent)),
		std::type_index(typeid(pg::RotationComponent)),
		std::type_index(typeid(pg::ScaleComponent)),
		std::type_index(typeid(pg::LocalBoundsComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::WorldTransformComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::WorldTransformComponent)),
	};
	return decl;
}

void pg::TransformComposeSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto view = accessor.View<const pg::PositionComponent, const pg::RotationComponent, const pg::ScaleComponent, const pg::LocalBoundsComponent>();
	for (pg::ecs::Entity ent : view)
	{
		const pg::PositionComponent& position = view.get<const pg::PositionComponent>(ent);
		const pg::RotationComponent& rotation = view.get<const pg::RotationComponent>(ent);
		const pg::ScaleComponent& scale = view.get<const pg::ScaleComponent>(ent);
		const pg::LocalBoundsComponent& bounds = view.get<const pg::LocalBoundsComponent>(ent);

		const glm::mat4 matrix = ComposeMatrix(position, rotation, scale);
		const float sortKey = (matrix * glm::vec4(bounds.m_Min, 1.f)).y;

		if (accessor.AllOf<pg::WorldTransformComponent>(ent))
		{
			pg::WorldTransformComponent& world = accessor.Get<pg::WorldTransformComponent>(ent);
			world.m_Matrix = matrix;
			world.m_SortKey = sortKey;
		}
		else
		{
			pg::WorldTransformComponent world;
			world.m_Matrix = matrix;
			world.m_SortKey = sortKey;
			accessor.EmplaceDeferred<pg::WorldTransformComponent>(ent, std::move(world));
		}
	}
}
