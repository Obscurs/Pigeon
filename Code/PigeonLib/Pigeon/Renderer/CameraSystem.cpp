#include "pch.h"
#include "Pigeon/Renderer/CameraSystem.h"

#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"

pg::SystemAccessDecl pg::CameraSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::KeyPressedEventComponent)),
		std::type_index(typeid(pg::MouseScrolledEventComponent)),
		std::type_index(typeid(pg::WindowResizeEventComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::OrthographicCameraComponent)),
	};
	return decl;
}

void pg::CameraSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto viewKeyPressedEvents = accessor.view<const pg::KeyPressedEventComponent>();
	auto viewScrollEvents = accessor.view<const pg::MouseScrolledEventComponent>();
	auto viewResizeEvents = accessor.view<const pg::WindowResizeEventComponent>();

	if (!viewKeyPressedEvents.empty() ||
		!viewScrollEvents.empty() ||
		!viewResizeEvents.empty()
		)
	{
		auto view = accessor.view<pg::OrthographicCameraComponent>();

		for (auto ent : view)
		{
			pg::OrthographicCameraComponent& component = view.get<pg::OrthographicCameraComponent>(ent);

			// Resize is a viewport event, not input: aspect ratio must stay in sync for all cameras.
			bool aspectDirty = false;
			for (auto e : viewResizeEvents)
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
				// Scroll first so speed sync below uses the updated zoom level.
				for (auto e : viewScrollEvents)
				{
					const pg::MouseScrolledEventComponent& eventComponent = viewScrollEvents.get<const pg::MouseScrolledEventComponent>(e);
					component.m_ZoomLevel -= eventComponent.m_YOffset * 0.25f;
					component.m_ZoomLevel = std::max(component.m_ZoomLevel, 0.25f);
				}
				component.m_CameraTranslationSpeed = component.m_ZoomLevel;

				for (auto e : viewKeyPressedEvents)
				{
					const pg::KeyPressedEventComponent& eventComponent = viewKeyPressedEvents.get<const pg::KeyPressedEventComponent>(e);
					if (eventComponent.m_KeyCode == pg::PG_KEY_A)
					{
						component.m_CameraPosition.x -= component.m_CameraTranslationSpeed * ts.AsSeconds();
					}
					else if (eventComponent.m_KeyCode == pg::PG_KEY_D)
					{
						component.m_CameraPosition.x += component.m_CameraTranslationSpeed * ts.AsSeconds();
					}
					else if (eventComponent.m_KeyCode == pg::PG_KEY_W)
					{
						component.m_CameraPosition.y += component.m_CameraTranslationSpeed * ts.AsSeconds();
					}
					else if (eventComponent.m_KeyCode == pg::PG_KEY_S)
					{
						component.m_CameraPosition.y -= component.m_CameraTranslationSpeed * ts.AsSeconds();
					}
				}
				component.m_Camera.SetPosition(component.m_CameraPosition);
			}
			else if (aspectDirty)
			{
				// Sync internal position before rebuilding the projection for non-reactive cameras.
				component.m_Camera.SetPosition(component.m_CameraPosition);
			}

			if (component.m_ReactsToInput || aspectDirty)
			{
				component.m_Camera.SetProjection(-component.m_AspectRatio * component.m_ZoomLevel, component.m_AspectRatio * component.m_ZoomLevel, -component.m_ZoomLevel, component.m_ZoomLevel);
			}
		}
	}
}
