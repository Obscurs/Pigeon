#include "pch.h"
#include "Pigeon/Renderer/CameraSystem.h"

#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Renderer/SetCameraRequestOneFrameComponent.h"
#include "Pigeon/Transform/CameraTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/PositionComponent.h"

pg::SystemAccessDecl pg::CameraSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::SetCameraRequestOneFrameComponent)),
		std::type_index(typeid(pg::WindowResizeEventComponent)),
		std::type_index(typeid(pg::PositionComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::OrthographicCameraComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::CameraTransformRequestOneFrameComponent)),
	};
	return decl;
}

void pg::CameraSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	// Input detection lives in the app, which emits a SetCameraRequest with the new zoom + position.
	// The last request of the frame wins.
	auto requestView = accessor.View<const pg::SetCameraRequestOneFrameComponent>();
	bool hasRequest = false;
	pg::SetCameraRequestOneFrameComponent request;
	for (pg::ecs::Entity e : requestView)
	{
		request = requestView.get<const pg::SetCameraRequestOneFrameComponent>(e);
		hasRequest = true;
	}

	auto viewResizeEvents = accessor.View<const pg::WindowResizeEventComponent>();

	auto view = accessor.View<pg::OrthographicCameraComponent>();
	for (pg::ecs::Entity ent : view)
	{
		pg::OrthographicCameraComponent& component = view.get<pg::OrthographicCameraComponent>(ent);

		// The resolved world position is the single source of truth for the camera location.
		glm::vec3 currentPosition(0.f, 0.f, 0.f);
		if (accessor.AllOf<pg::PositionComponent>(ent))
		{
			currentPosition = accessor.Get<const pg::PositionComponent>(ent).m_Position;
		}

		// Resize is a viewport event, not input: aspect ratio must stay in sync for all cameras.
		bool aspectDirty = false;
		for (pg::ecs::Entity e : viewResizeEvents)
		{
			const pg::WindowResizeEventComponent& eventComponent = viewResizeEvents.get<const pg::WindowResizeEventComponent>(e);
			if (eventComponent.m_Height > 0)
			{
				component.m_AspectRatio = (float)eventComponent.m_Width / (float)eventComponent.m_Height;
				aspectDirty = true;
			}
		}

		// Apply the app's new zoom to the camera controller and pan its PositionComponent via the
		// transform pipeline (the sole writer of PositionComponent).
		if (hasRequest)
		{
			component.m_ZoomLevel = request.m_ZoomLevel;

			pg::CameraTransformRequestOneFrameComponent panRequest;
			panRequest.m_Data.m_SetPosition = true;
			panRequest.m_Data.m_Position = request.m_Position;
			accessor.EmplaceOneframe<pg::CameraTransformRequestOneFrameComponent>(ent, std::move(panRequest));
		}

		// Keep the internal camera in sync with the resolved position every frame; rebuild the
		// projection only when the zoom or aspect could have changed.
		component.m_Camera.SetPosition(currentPosition);
		if (hasRequest || aspectDirty)
		{
			component.m_Camera.SetProjection(-component.m_AspectRatio * component.m_ZoomLevel, component.m_AspectRatio * component.m_ZoomLevel, -component.m_ZoomLevel, component.m_ZoomLevel);
		}
	}
}
