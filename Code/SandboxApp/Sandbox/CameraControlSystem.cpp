#include "Sandbox/CameraControlSystem.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Renderer/SetCameraRequestOneFrameComponent.h"
#include "Pigeon/Transform/PositionComponent.h"

#include <algorithm>

namespace
{
	bool IsKeyHeld(const pg::InputStateSingletonComponent& input, int key)
	{
		return input.m_KeysPressed.count(key) > 0;
	}
}

pg::SystemAccessDecl sbx::CameraControlSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::InputStateSingletonComponent)),
		std::type_index(typeid(pg::MouseScrolledEventComponent)),
		std::type_index(typeid(pg::OrthographicCameraComponent)),
		std::type_index(typeid(pg::PositionComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::SetCameraRequestOneFrameComponent)),
	};
	return decl;
}

void sbx::CameraControlSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	auto inputView = accessor.View<const pg::InputStateSingletonComponent>();
	if (inputView.empty())
	{
		return;
	}
	const pg::InputStateSingletonComponent& input = inputView.get<const pg::InputStateSingletonComponent>(inputView.front());

	auto cameraView = accessor.View<const pg::OrthographicCameraComponent>();
	if (cameraView.empty())
	{
		return;
	}
	pg::ecs::Entity cameraEnt = cameraView.front();
	const pg::OrthographicCameraComponent& camera = cameraView.get<const pg::OrthographicCameraComponent>(cameraEnt);

	// Scroll drives zoom. The InputState scroll value is sticky (never reset to zero), so read the
	// per-frame scroll event directly instead.
	auto scrollView = accessor.View<const pg::MouseScrolledEventComponent>();
	float scrollY = 0.f;
	for (pg::ecs::Entity e : scrollView)
	{
		scrollY += scrollView.get<const pg::MouseScrolledEventComponent>(e).m_YOffset;
	}

	const float panX = (IsKeyHeld(input, pg::PG_KEY_D) ? 1.f : 0.f) - (IsKeyHeld(input, pg::PG_KEY_A) ? 1.f : 0.f);
	const float panY = (IsKeyHeld(input, pg::PG_KEY_W) ? 1.f : 0.f) - (IsKeyHeld(input, pg::PG_KEY_S) ? 1.f : 0.f);

	if (scrollY == 0.f && panX == 0.f && panY == 0.f)
	{
		return;
	}

	// Zoom in/out (clamped); pan speed scales with the zoom level so a zoomed-in view pans slower.
	const float zoom = std::max(camera.m_ZoomLevel - scrollY * 0.25f, 0.25f);

	glm::vec3 position(0.f, 0.f, 0.f);
	if (accessor.AllOf<pg::PositionComponent>(cameraEnt))
	{
		position = accessor.Get<const pg::PositionComponent>(cameraEnt).m_Position;
	}
	position.x += panX * zoom * ts.AsSeconds();
	position.y += panY * zoom * ts.AsSeconds();

	pg::SetCameraRequestOneFrameComponent request;
	request.m_ZoomLevel = zoom;
	request.m_Position = position;
	pg::ecs::Entity requestEnt = accessor.Create();
	accessor.EmplaceOneframe<pg::SetCameraRequestOneFrameComponent>(requestEnt, std::move(request));
}
