#include "Sandbox/QuadAnimationSystem.h"

#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/TransformRequestData.h"
#include "Sandbox/AnimationTransformRequestOneFrameComponent.h"
#include "Sandbox/DebugControlsSingletonComponent.h"
#include "Sandbox/QuadComponent.h"
#include "Sandbox/SpinComponent.h"

#include <cmath>
#include <glm/gtc/quaternion.hpp>
#include <utility>

namespace
{
	float GetAnimationSpeed(pg::CheckedRegistryAccessor& accessor)
	{
		auto view = accessor.View<const sbx::DebugControlsSingletonComponent>();
		if (view.empty())
		{
			return 1.f;
		}
		return view.get<const sbx::DebugControlsSingletonComponent>(view.front()).m_AnimationSpeed;
	}

	glm::vec3 ComputeColor(const sbx::SpinComponent& spin)
	{
		if (spin.m_ColorCycleSpeed == 0.f)
		{
			return spin.m_BaseColor;
		}

		// Three sine waves 120 degrees apart give a smooth hue cycle in [0,1] per channel.
		const float t = spin.m_Elapsed * spin.m_ColorCycleSpeed;
		return glm::vec3(
			0.5f + 0.5f * std::sin(t),
			0.5f + 0.5f * std::sin(t + 2.0944f),
			0.5f + 0.5f * std::sin(t + 4.1888f));
	}

	glm::vec3 ComputePosition(const sbx::SpinComponent& spin)
	{
		glm::vec3 position = spin.m_Anchor;
		if (spin.m_OrbitRadius != 0.f)
		{
			const float orbitAngle = spin.m_Elapsed * spin.m_OrbitSpeed;
			position.x += std::cos(orbitAngle) * spin.m_OrbitRadius;
			position.y += std::sin(orbitAngle) * spin.m_OrbitRadius;
		}
		return position;
	}

	glm::quat ComputeRotation(const sbx::SpinComponent& spin)
	{
		if (spin.m_RotationSpeed == 0.f)
		{
			return glm::quat(1.f, 0.f, 0.f, 0.f);
		}
		return glm::angleAxis(spin.m_Elapsed * spin.m_RotationSpeed, glm::vec3(0.f, 0.f, 1.f));
	}
}

pg::SystemAccessDecl sbx::QuadAnimationSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(sbx::DebugControlsSingletonComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(sbx::SpinComponent)),
		std::type_index(typeid(sbx::QuadComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::AnimationTransformRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::QuadAnimationSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	const float deltaSeconds = ts.AsSeconds() * GetAnimationSpeed(accessor);

	auto view = accessor.View<sbx::SpinComponent, sbx::QuadComponent>();
	for (pg::ecs::Entity ent : view)
	{
		sbx::SpinComponent& spin = view.get<sbx::SpinComponent>(ent);
		sbx::QuadComponent& quad = view.get<sbx::QuadComponent>(ent);

		spin.m_Elapsed += deltaSeconds;
		quad.m_Color = ComputeColor(spin);

		sbx::AnimationTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = ComputePosition(spin);
		request.m_Data.m_SetRotation = true;
		request.m_Data.m_Rotation = ComputeRotation(spin);
		accessor.EmplaceOneframe<sbx::AnimationTransformRequestOneFrameComponent>(ent, std::move(request));
	}
}
