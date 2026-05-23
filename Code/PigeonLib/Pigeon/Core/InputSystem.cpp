#include "pch.h"
#include "InputSystem.h"

#include <imgui.h>

#include "Pigeon/Core/Application.h"
#include "Pigeon/Core/Input.h"
#include "Pigeon/Core/InputComponents.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/KeyReleasedEventComponent.h"
#include "Pigeon/Core/KeyTypedEventComponent.h"
#include "Pigeon/Core/MouseButtonPressedEventComponent.h"
#include "Pigeon/Core/MouseButtonReleasedEventComponent.h"
#include "Pigeon/Core/MouseMovedEventComponent.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Events/MouseEvent.h"
#include "Pigeon/Events/KeyEvent.h"

namespace
{
	struct InputEvent
	{
		pig::EventType m_Type = pig::EventType::None;
		int m_KeyCode = 0;
		float m_FloatData1 = 0;
		float m_FloatData2 = 0;
	};

	void ProcessEvents(const std::vector<InputEvent>& events, pig::InputStateSingletonComponent& inputState)
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
			if (inputEvent.m_Type == pig::EventType::KeyPressed || inputEvent.m_Type == pig::EventType::MouseButtonPressed)
			{
				inputState.m_KeysPressed[inputEvent.m_KeyCode] = 1;
			}
			else if (inputEvent.m_Type == pig::EventType::KeyTyped)
			{
				inputState.m_KeysTyped.push_back(inputEvent.m_KeyCode);
			}
			else if (inputEvent.m_Type == pig::EventType::KeyReleased || inputEvent.m_Type == pig::EventType::MouseButtonReleased)
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
			else if (inputEvent.m_Type == pig::EventType::MouseMoved)
			{
				inputState.m_MousePos.x = inputEvent.m_FloatData1;
				inputState.m_MousePos.y = inputEvent.m_FloatData2;
			}
			else if (inputEvent.m_Type == pig::EventType::MouseScrolled)
			{
				inputState.m_MouseScroll.x = inputEvent.m_FloatData1;
				inputState.m_MouseScroll.y = inputEvent.m_FloatData2;
			}
		}
	}
}

pig::SystemAccessDecl pig::InputSystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;
	decl.readSet = {
		std::type_index(typeid(pig::KeyPressedEventComponent)),
		std::type_index(typeid(pig::KeyReleasedEventComponent)),
		std::type_index(typeid(pig::KeyTypedEventComponent)),
		std::type_index(typeid(pig::MouseMovedEventComponent)),
		std::type_index(typeid(pig::MouseButtonPressedEventComponent)),
		std::type_index(typeid(pig::MouseButtonReleasedEventComponent)),
		std::type_index(typeid(pig::MouseScrolledEventComponent)),
	};
	decl.writeSet = {
		std::type_index(typeid(pig::InputStateSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pig::InputStateSingletonComponent)),
	};
	return decl;
}

void pig::InputSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	auto view = accessor.view<pig::InputStateSingletonComponent>();
	if (view.empty())
	{
		pig::InputStateSingletonComponent component;
		entt::entity entity = accessor.create();
		accessor.emplace_deferred<pig::InputStateSingletonComponent>(entity, std::move(component));
	}
	else
	{
		std::vector<InputEvent> events;
		auto viewKeyPressedEvents = accessor.view<const pig::KeyPressedEventComponent>();
		auto viewKeyReleasedEvents = accessor.view<const pig::KeyReleasedEventComponent>();
		auto viewKeyTypedEvents = accessor.view<const pig::KeyTypedEventComponent>();
		auto viewMouseMovedEvents = accessor.view<const pig::MouseMovedEventComponent>();
		auto viewMousePressedEvents = accessor.view<const pig::MouseButtonPressedEventComponent>();
		auto viewMouseReleasedEvents = accessor.view<const pig::MouseButtonReleasedEventComponent>();
		auto viewMouseScrolledEvents = accessor.view<const pig::MouseScrolledEventComponent>();

		pig::InputStateSingletonComponent& component = view.get<pig::InputStateSingletonComponent>(view.front());

		for (auto e : viewKeyPressedEvents)
		{
			const pig::KeyPressedEventComponent& eventComponent = viewKeyPressedEvents.get<const pig::KeyPressedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pig::EventType::KeyPressed;
			inputEvent.m_KeyCode = pig::PlatformInput::GetKeyCode(eventComponent.m_KeyCode);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewKeyReleasedEvents)
		{
			const pig::KeyReleasedEventComponent& eventComponent = viewKeyReleasedEvents.get<const pig::KeyReleasedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pig::EventType::KeyReleased;
			inputEvent.m_KeyCode = pig::PlatformInput::GetKeyCode(eventComponent.m_KeyCode);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewKeyTypedEvents)
		{
			const pig::KeyTypedEventComponent& eventComponent = viewKeyTypedEvents.get<const pig::KeyTypedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pig::EventType::KeyTyped;
			inputEvent.m_KeyCode = pig::PlatformInput::GetKeyCode(eventComponent.m_KeyCode);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewMouseMovedEvents)
		{
			const pig::MouseMovedEventComponent& eventComponent = viewMouseMovedEvents.get<const pig::MouseMovedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pig::EventType::MouseMoved;
			inputEvent.m_FloatData1 = eventComponent.m_MouseX;
			inputEvent.m_FloatData2 = eventComponent.m_MouseY;
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewMousePressedEvents)
		{
			const pig::MouseButtonPressedEventComponent& eventComponent = viewMousePressedEvents.get<const pig::MouseButtonPressedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pig::EventType::MouseButtonPressed;
			inputEvent.m_KeyCode = pig::PlatformInput::GetMouseButtonCode(eventComponent.m_Button);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewMouseReleasedEvents)
		{
			const pig::MouseButtonReleasedEventComponent& eventComponent = viewMouseReleasedEvents.get<const pig::MouseButtonReleasedEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pig::EventType::MouseButtonReleased;
			inputEvent.m_KeyCode = pig::PlatformInput::GetMouseButtonCode(eventComponent.m_Button);
			events.push_back(std::move(inputEvent));
		}
		for (auto e : viewMouseScrolledEvents)
		{
			const pig::MouseScrolledEventComponent& eventComponent = viewMouseScrolledEvents.get<const pig::MouseScrolledEventComponent>(e);
			InputEvent inputEvent;
			inputEvent.m_Type = pig::EventType::MouseScrolled;
			inputEvent.m_FloatData1 = eventComponent.m_XOffset;
			inputEvent.m_FloatData2 = eventComponent.m_YOffset;
			events.push_back(std::move(inputEvent));
		}
		ProcessEvents(events, component);
	}
}
