#include "pch.h"
#include "Pigeon/Core/InputSystem.h"

#include "Pigeon/Core/Application.h"
#include "Pigeon/Core/Input.h"
#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/KeyReleasedEventComponent.h"
#include "Pigeon/Core/KeyTypedEventComponent.h"
#include "Pigeon/Core/MouseButtonPressedEventComponent.h"
#include "Pigeon/Core/MouseButtonReleasedEventComponent.h"
#include "Pigeon/Core/MouseMovedEventComponent.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Events/KeyEvent.h"
#include "Pigeon/Events/MouseEvent.h"
#include <imgui.h>

namespace
{
	struct InputEvent
	{
		pg::EventType m_Type = pg::EventType::None;
		int m_KeyCode = 0;
		float m_FloatData1 = 0;
		float m_FloatData2 = 0;
	};

	void ProcessEvents(const std::vector<InputEvent>& events, pg::InputStateSingletonComponent& inputState)
	{
		inputState.m_KeysReleased.clear();
		inputState.m_KeysTyped.clear();
		for (auto it = inputState.m_KeysPressed.begin(); it != inputState.m_KeysPressed.end(); ++it)
		{
			++it->second;
		}
		for (int i = 0; i < events.size(); ++i)
		{
			const InputEvent& inputEvent = events[i];
			if (inputEvent.m_Type == pg::EventType::KeyPressed || inputEvent.m_Type == pg::EventType::MouseButtonPressed)
			{
				inputState.m_KeysPressed[inputEvent.m_KeyCode] = 1;
			}
			else if (inputEvent.m_Type == pg::EventType::KeyTyped)
			{
				inputState.m_KeysTyped.push_back(inputEvent.m_KeyCode);
			}
			else if (inputEvent.m_Type == pg::EventType::KeyReleased || inputEvent.m_Type == pg::EventType::MouseButtonReleased)
			{
				auto it = inputState.m_KeysPressed.find(inputEvent.m_KeyCode);
				if (it != inputState.m_KeysPressed.end())
				{
					inputState.m_KeysReleased[inputEvent.m_KeyCode] = it->second;
					inputState.m_KeysPressed.erase(inputEvent.m_KeyCode);
				}
				else
				{
					inputState.m_KeysReleased[inputEvent.m_KeyCode] = 0;
				}
			}
			else if (inputEvent.m_Type == pg::EventType::MouseMoved)
			{
				inputState.m_MousePos.x = inputEvent.m_FloatData1;
				inputState.m_MousePos.y = inputEvent.m_FloatData2;
			}
			else if (inputEvent.m_Type == pg::EventType::MouseScrolled)
			{
				inputState.m_MouseScroll.x = inputEvent.m_FloatData1;
				inputState.m_MouseScroll.y = inputEvent.m_FloatData2;
			}
		}
	}
}

pg::SystemAccessDecl pg::InputSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pg::KeyPressedEventComponent)),
		std::type_index(typeid(pg::KeyReleasedEventComponent)),
		std::type_index(typeid(pg::KeyTypedEventComponent)),
		std::type_index(typeid(pg::MouseMovedEventComponent)),
		std::type_index(typeid(pg::MouseButtonPressedEventComponent)),
		std::type_index(typeid(pg::MouseButtonReleasedEventComponent)),
		std::type_index(typeid(pg::MouseScrolledEventComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pg::InputStateSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::InputStateSingletonComponent)),
	};
	return decl;
}

void pg::InputSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	auto view = accessor.view<pg::InputStateSingletonComponent>();
	if (view.empty())
	{
		pg::InputStateSingletonComponent component;
		pg::ecs::Entity entity = accessor.create();
		accessor.emplace_deferred<pg::InputStateSingletonComponent>(entity, std::move(component));
	}
	else
	{
		std::vector<InputEvent> events;
		auto viewKeyPressedEvents = accessor.view<const pg::KeyPressedEventComponent>();
		auto viewKeyReleasedEvents = accessor.view<const pg::KeyReleasedEventComponent>();
		auto viewKeyTypedEvents = accessor.view<const pg::KeyTypedEventComponent>();
		auto viewMouseMovedEvents = accessor.view<const pg::MouseMovedEventComponent>();
		auto viewMousePressedEvents = accessor.view<const pg::MouseButtonPressedEventComponent>();
		auto viewMouseReleasedEvents = accessor.view<const pg::MouseButtonReleasedEventComponent>();
		auto viewMouseScrolledEvents = accessor.view<const pg::MouseScrolledEventComponent>();

		pg::InputStateSingletonComponent& component = view.get<pg::InputStateSingletonComponent>(view.front());

		for (auto e : viewKeyPressedEvents)
		{
			const pg::KeyPressedEventComponent& eventComponent = viewKeyPressedEvents.get<const pg::KeyPressedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pg::EventType::KeyPressed;
			inputEvent.m_KeyCode = pg::PlatformInput::GetKeyCode(eventComponent.m_KeyCode);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewKeyReleasedEvents)
		{
			const pg::KeyReleasedEventComponent& eventComponent = viewKeyReleasedEvents.get<const pg::KeyReleasedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pg::EventType::KeyReleased;
			inputEvent.m_KeyCode = pg::PlatformInput::GetKeyCode(eventComponent.m_KeyCode);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewKeyTypedEvents)
		{
			const pg::KeyTypedEventComponent& eventComponent = viewKeyTypedEvents.get<const pg::KeyTypedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pg::EventType::KeyTyped;
			inputEvent.m_KeyCode = pg::PlatformInput::GetKeyCode(eventComponent.m_KeyCode);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewMouseMovedEvents)
		{
			const pg::MouseMovedEventComponent& eventComponent = viewMouseMovedEvents.get<const pg::MouseMovedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pg::EventType::MouseMoved;
			inputEvent.m_FloatData1 = eventComponent.m_MouseX;
			inputEvent.m_FloatData2 = eventComponent.m_MouseY;
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewMousePressedEvents)
		{
			const pg::MouseButtonPressedEventComponent& eventComponent = viewMousePressedEvents.get<const pg::MouseButtonPressedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pg::EventType::MouseButtonPressed;
			inputEvent.m_KeyCode = pg::PlatformInput::GetMouseButtonCode(eventComponent.m_Button);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewMouseReleasedEvents)
		{
			const pg::MouseButtonReleasedEventComponent& eventComponent = viewMouseReleasedEvents.get<const pg::MouseButtonReleasedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pg::EventType::MouseButtonReleased;
			inputEvent.m_KeyCode = pg::PlatformInput::GetMouseButtonCode(eventComponent.m_Button);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewMouseScrolledEvents)
		{
			const pg::MouseScrolledEventComponent& eventComponent = viewMouseScrolledEvents.get<const pg::MouseScrolledEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pg::EventType::MouseScrolled;
			inputEvent.m_FloatData1 = eventComponent.m_XOffset;
			inputEvent.m_FloatData2 = eventComponent.m_YOffset;
			events.push_back(std::move(inputEvent));
		}
		ProcessEvents(events, component);
	}
}
