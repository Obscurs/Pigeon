#include "pch.h"
#include "CameraSystem.h"

#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/OrthographicCameraComponent.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/World.h"

pig::SystemAccessDecl pig::CameraSystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pig::KeyPressedEventComponent)),
		std::type_index(typeid(pig::MouseScrolledEventComponent)),
		std::type_index(typeid(pig::WindowResizeEventComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pig::OrthographicCameraComponent)),
	};
	return decl;
}

void pig::CameraSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	auto viewKeyPressedEvents = accessor.view<const pig::KeyPressedEventComponent>();
	auto viewScrollEvents = accessor.view<const pig::MouseScrolledEventComponent>();
	auto viewResizeEvents = accessor.view<const pig::WindowResizeEventComponent>();
	
	if (!viewKeyPressedEvents.empty() ||
		!viewScrollEvents.empty() ||
		!viewResizeEvents.empty()
		)
	{
		auto view = accessor.view<pig::OrthographicCameraComponent>();

		for (auto ent : view)
		{
			pig::OrthographicCameraComponent& component = view.get<pig::OrthographicCameraComponent>(ent);

			if (component.m_ReactsToInput)
			{
				for (auto e : viewKeyPressedEvents)
				{
					const pig::KeyPressedEventComponent& eventComponent = viewKeyPressedEvents.get<const pig::KeyPressedEventComponent>(e);
					if (eventComponent.m_KeyCode == pig::PG_KEY_A)
					{
						component.m_CameraPosition.x -= component.m_CameraTranslationSpeed * ts.AsSeconds();
					}
					else if (eventComponent.m_KeyCode == pig::PG_KEY_D)
					{
						component.m_CameraPosition.x += component.m_CameraTranslationSpeed * ts.AsSeconds();
					}
					else if (eventComponent.m_KeyCode == pig::PG_KEY_W)
					{
						component.m_CameraPosition.y += component.m_CameraTranslationSpeed * ts.AsSeconds();
					}
					else if (eventComponent.m_KeyCode == pig::PG_KEY_S)
					{
						component.m_CameraPosition.y -= component.m_CameraTranslationSpeed * ts.AsSeconds();
					}
				}
				for (auto e : viewScrollEvents)
				{
					const pig::MouseScrolledEventComponent& eventComponent = viewScrollEvents.get<const pig::MouseScrolledEventComponent>(e);
					component.m_ZoomLevel -= eventComponent.m_YOffset * 0.25f;
					component.m_ZoomLevel = std::max(component.m_ZoomLevel, 0.25f);
				}
				for (auto e : viewResizeEvents)
				{
					const pig::WindowResizeEventComponent& eventComponent = viewResizeEvents.get<const pig::WindowResizeEventComponent>(e);
					component.m_AspectRatio = (float)eventComponent.m_Width / (float)eventComponent.m_Height;
				}
			}

			component.m_Camera.SetPosition(component.m_CameraPosition);

			component.m_CameraTranslationSpeed = component.m_ZoomLevel;
			component.m_Camera.SetProjection(-component.m_AspectRatio * component.m_ZoomLevel, component.m_AspectRatio * component.m_ZoomLevel, -component.m_ZoomLevel, component.m_ZoomLevel);
		}
	}
}
