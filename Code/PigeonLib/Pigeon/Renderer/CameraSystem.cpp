#include "pch.h"
#include "Pigeon/Renderer/CameraSystem.h"

#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Transform/CameraTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/PositionComponent.h"

pg::SystemAccessDecl pg::CameraSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::KeyPressedEventComponent)),
		std::type_index(typeid(pg::MouseScrolledEventComponent)),
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

	auto viewKeyPressedEvents = accessor.View<const pg::KeyPressedEventComponent>();
	auto viewScrollEvents = accessor.View<const pg::MouseScrolledEventComponent>();
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

		if (component.m_ReactsToInput)
		{
			// Scroll first so the translation-speed sync below uses the updated zoom level.
			for (pg::ecs::Entity e : viewScrollEvents)
			{
				const pg::MouseScrolledEventComponent& eventComponent = viewScrollEvents.get<const pg::MouseScrolledEventComponent>(e);
				component.m_ZoomLevel -= eventComponent.m_YOffset * 0.25f;
				component.m_ZoomLevel = std::max(component.m_ZoomLevel, 0.25f);
			}
			component.m_CameraTranslationSpeed = component.m_ZoomLevel;

			glm::vec3 pannedPosition = currentPosition;
			bool panned = false;
			for (pg::ecs::Entity e : viewKeyPressedEvents)
			{
				const pg::KeyPressedEventComponent& eventComponent = viewKeyPressedEvents.get<const pg::KeyPressedEventComponent>(e);
				if (eventComponent.m_KeyCode == pg::PG_KEY_A)
				{
					pannedPosition.x -= component.m_CameraTranslationSpeed * ts.AsSeconds();
					panned = true;
				}
				else if (eventComponent.m_KeyCode == pg::PG_KEY_D)
				{
					pannedPosition.x += component.m_CameraTranslationSpeed * ts.AsSeconds();
					panned = true;
				}
				else if (eventComponent.m_KeyCode == pg::PG_KEY_W)
				{
					pannedPosition.y += component.m_CameraTranslationSpeed * ts.AsSeconds();
					panned = true;
				}
				else if (eventComponent.m_KeyCode == pg::PG_KEY_S)
				{
					pannedPosition.y -= component.m_CameraTranslationSpeed * ts.AsSeconds();
					panned = true;
				}
			}

			if (panned)
			{
				pg::CameraTransformRequestOneFrameComponent request;
				request.m_Data.m_SetPosition = true;
				request.m_Data.m_Position = pannedPosition;
				accessor.EmplaceOneframe<pg::CameraTransformRequestOneFrameComponent>(ent, std::move(request));
			}
		}

		// Keep the internal camera in sync with the resolved position every frame; rebuild the
		// projection only when the zoom or aspect could have changed.
		component.m_Camera.SetPosition(currentPosition);
		if (component.m_ReactsToInput || aspectDirty)
		{
			component.m_Camera.SetProjection(-component.m_AspectRatio * component.m_ZoomLevel, component.m_AspectRatio * component.m_ZoomLevel, -component.m_ZoomLevel, component.m_ZoomLevel);
		}
	}
}
