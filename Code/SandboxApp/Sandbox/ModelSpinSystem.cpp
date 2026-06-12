#include "Sandbox/ModelSpinSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/TransformRequestData.h"
#include "Sandbox/ModelSpinComponent.h"
#include "Sandbox/ModelSpinTransformRequestOneFrameComponent.h"

#include <glm/gtc/quaternion.hpp>
#include <utility>

pg::SystemAccessDecl sbx::ModelSpinSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.writeSet = {
		std::type_index(typeid(sbx::ModelSpinComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::ModelSpinTransformRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::ModelSpinSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	const float deltaSeconds = ts.AsSeconds();

	auto view = accessor.View<sbx::ModelSpinComponent>();
	for (pg::ecs::Entity ent : view)
	{
		sbx::ModelSpinComponent& spin = view.get<sbx::ModelSpinComponent>(ent);
		spin.m_Elapsed += deltaSeconds;

		sbx::ModelSpinTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = spin.m_Anchor;
		request.m_Data.m_SetRotation = true;
		request.m_Data.m_Rotation = glm::angleAxis(spin.m_Elapsed * spin.m_RotationSpeed, glm::vec3(0.f, 1.f, 0.f));
		accessor.EmplaceOneframe<sbx::ModelSpinTransformRequestOneFrameComponent>(ent, std::move(request));
	}
}
