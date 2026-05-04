#include "pch.h"
#include "InputLayer.h"

#include <imgui.h>

#include "Pigeon/Core/Application.h"
#include "Pigeon/Core/Input.h"
#include "Pigeon/Core/InputComponents.h"
#include "Pigeon/Core/KeyCodes.h"

#include "Pigeon/ECS/World.h"

#include <Pigeon/Events/MouseEvent.h>
#include <Pigeon/Events/KeyEvent.h>

pig::S_Ptr<pig::Input> pig::Input::s_Instance = std::make_shared<pig::Input>();
pig::InputLayer::InputLayer()
	: Layer("InputLayer")
{
}

bool pig::InputLayer::OnEvent(const pig::Event& e)
{
	pig::EventDispatcher::Dispatch<pig::KeyPressedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendKeyEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::KeyReleasedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendKeyEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::KeyTypedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendKeyTypedEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::MouseMovedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendMouseMoveEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::MouseButtonPressedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendMouseButtonEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::MouseButtonReleasedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendMouseButtonEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::MouseScrolledEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendMouseButtonEvent, pig::InputLayer>(this));

	return false;
}

void pig::InputLayer::OnUpdate(const pig::Timestep& ts)
{
	auto view = pig::World::GetRegistry().view<pig::InputStateSingletonComponent>();
	if (view.size() == 0)
	{
		pig::World::GetRegistry().emplace<pig::InputStateSingletonComponent>(pig::World::GetRegistry().create());
		return;
	}

	PG_CORE_ASSERT(view.size() == 1, "There should only be one ui render config component");
	pig::InputStateSingletonComponent& inputState = view.get<pig::InputStateSingletonComponent>(view.front());

	ProcessEvents(inputState);
}

bool pig::InputLayer::IsKeyTyped(int keycode) const
{
	auto view = pig::World::GetRegistry().view<const pig::InputStateSingletonComponent>();
	if (view.size() == 1)
	{
		const pig::InputStateSingletonComponent& inputState = view.get<const pig::InputStateSingletonComponent>(view.front());
		for (int i = 0; i < inputState.m_KeysTyped.size(); ++i)
		{
			if (inputState.m_KeysTyped[i] == keycode)
				return true;
		}
	}
	
	return false;
}

bool pig::InputLayer::IsKeyPressed(int keycode, bool justPressed) const
{
	auto view = pig::World::GetRegistry().view<const pig::InputStateSingletonComponent>();
	if (view.size() == 1)
	{
		const pig::InputStateSingletonComponent& inputState = view.get<const pig::InputStateSingletonComponent>(view.front());
		auto it = inputState.m_KeysPressed.find(keycode);
		if (it == inputState.m_KeysPressed.end())
			return false;

		return justPressed ? it->second == 1 : true;
	}
	return false;
}

bool pig::InputLayer::IsKeyReleased(int keycode) const
{
	auto view = pig::World::GetRegistry().view<const pig::InputStateSingletonComponent>();
	if (view.size() == 1)
	{
		const pig::InputStateSingletonComponent& inputState = view.get<const pig::InputStateSingletonComponent>(view.front());
		return inputState.m_KeysReleased.find(keycode) != inputState.m_KeysReleased.end();
	}
	return false;
}

bool pig::InputLayer::IsMouseButtonPressed(int button, bool justPressed) const
{
	auto view = pig::World::GetRegistry().view<const pig::InputStateSingletonComponent>();
	if (view.size() == 1)
	{
		const pig::InputStateSingletonComponent& inputState = view.get<const pig::InputStateSingletonComponent>(view.front());
		auto it = inputState.m_KeysPressed.find(button);
		if (it == inputState.m_KeysPressed.end())
			return false;

		return justPressed ? it->second == 1 : true;
	}
	return false;
}

bool pig::InputLayer::IsMouseButtonReleased(int button) const
{
	auto view = pig::World::GetRegistry().view<const pig::InputStateSingletonComponent>();
	if (view.size() == 1)
	{
		const pig::InputStateSingletonComponent& inputState = view.get<const pig::InputStateSingletonComponent>(view.front());
		return inputState.m_KeysReleased.find(button) != inputState.m_KeysReleased.end();
	}
	return false;
}

glm::vec2 pig::InputLayer::GetMousePosition() const
{
	auto view = pig::World::GetRegistry().view<const pig::InputStateSingletonComponent>();
	if (view.size() == 1)
	{
		const pig::InputStateSingletonComponent& inputState = view.get<const pig::InputStateSingletonComponent>(view.front());
		return inputState.m_MousePos;
	}
	return glm::vec2{};
}

glm::vec2 pig::InputLayer::GetMouseScrolled() const
{
	auto view = pig::World::GetRegistry().view<const pig::InputStateSingletonComponent>();
	if (view.size() == 1)
	{
		const pig::InputStateSingletonComponent& inputState = view.get<const pig::InputStateSingletonComponent>(view.front());
		return inputState.m_MouseScroll;
	}
	return glm::vec2{};
}

std::vector<int> pig::InputLayer::GetKeysTyped() const
{
	auto view = pig::World::GetRegistry().view<const pig::InputStateSingletonComponent>();
	if (view.size() == 1)
	{
		const pig::InputStateSingletonComponent& inputState = view.get<const pig::InputStateSingletonComponent>(view.front());
		return inputState.m_KeysTyped;
	}
	return std::vector<int>();
}

bool pig::InputLayer::AppendKeyEvent(const pig::Event& e)
{
	InputEvent inputEvent;
	inputEvent.m_Type = e.GetEventType();

	switch (inputEvent.m_Type)
	{
	case pig::EventType::KeyPressed:
	case pig::EventType::KeyReleased:
		inputEvent.m_KeyCode = pig::PlatformInput::GetKeyCode(dynamic_cast<const pig::KeyEvent&>(e).GetKeyCode());
		break;
	default:
		PG_CORE_ASSERT(false, "event type not implemented");
	}
	if (inputEvent.m_KeyCode)
		m_Events.push_back(std::move(inputEvent));
	return false;
}

bool pig::InputLayer::AppendKeyTypedEvent(const pig::Event& e)
{
	InputEvent inputEvent;
	inputEvent.m_Type = e.GetEventType();

	switch (inputEvent.m_Type)
	{
	case pig::EventType::KeyTyped:
		inputEvent.m_KeyCode = dynamic_cast<const pig::KeyEvent&>(e).GetKeyCode();
		break;
	default:
		PG_CORE_ASSERT(false, "event type not implemented");
	}
	m_Events.push_back(std::move(inputEvent));
	return false;
}

bool pig::InputLayer::AppendMouseMoveEvent(const pig::Event& e)
{
	InputEvent inputEvent;
	inputEvent.m_Type = e.GetEventType();

	switch (inputEvent.m_Type)
	{
	case pig::EventType::MouseMoved:
		inputEvent.m_FloatData1 = dynamic_cast<const pig::MouseMovedEvent&>(e).GetX();
		inputEvent.m_FloatData2 = dynamic_cast<const pig::MouseMovedEvent&>(e).GetY();
		break;
	default:
		PG_CORE_ASSERT(false, "event type not implemented");
	}
	m_Events.push_back(std::move(inputEvent));
	return false;
}

bool pig::InputLayer::AppendMouseButtonEvent(const pig::Event& e)
{
	InputEvent inputEvent;
	inputEvent.m_Type = e.GetEventType();

	switch (inputEvent.m_Type)
	{
	case pig::EventType::MouseButtonPressed:
	case pig::EventType::MouseButtonReleased:
		inputEvent.m_KeyCode = pig::PlatformInput::GetMouseButtonCode(dynamic_cast<const pig::MouseButtonEvent&>(e).GetMouseButton());
		break;
	case pig::EventType::MouseScrolled:
		inputEvent.m_FloatData1 = dynamic_cast<const pig::MouseScrolledEvent&>(e).GetXOffset();
		inputEvent.m_FloatData2 = dynamic_cast<const pig::MouseScrolledEvent&>(e).GetYOffset();
		break;
	default:
		PG_CORE_ASSERT(false, "event type not implemented");
	}
	m_Events.push_back(std::move(inputEvent));
	return false;
}

void pig::InputLayer::ProcessEvents(pig::InputStateSingletonComponent& inputState)
{
	inputState.m_KeysReleased.clear();
	inputState.m_KeysTyped.clear();
	for (auto it = inputState.m_KeysPressed.begin(); it != inputState.m_KeysPressed.end(); ++it)
	{
		++it->second;
	}
	for (int i = 0; i < m_Events.size(); ++i)
	{
		const InputEvent& inputEvent = m_Events[i];
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
	m_Events.clear();
}

bool pig::Input::IsKeyTyped(int keycode)
{
	return pig::Application::Get().GetInputLayer().IsKeyTyped(keycode);
}

bool pig::Input::IsKeyPressed(int keycode, bool justPressed)
{
	return pig::Application::Get().GetInputLayer().IsKeyPressed(keycode, justPressed);
}

bool pig::Input::IsKeyReleased(int keycode)
{
	return pig::Application::Get().GetInputLayer().IsKeyReleased(keycode);
}

bool pig::Input::IsMouseButtonPressed(int button, bool justPressed)
{
	return pig::Application::Get().GetInputLayer().IsMouseButtonPressed(button, justPressed);
}

bool pig::Input::IsMouseButtonReleased(int button)
{
	return pig::Application::Get().GetInputLayer().IsMouseButtonReleased(button);
}

glm::vec2 pig::Input::GetMousePosition()
{
	return pig::Application::Get().GetInputLayer().GetMousePosition();
}

glm::vec2 pig::Input::GetMouseScrolled()
{
	return pig::Application::Get().GetInputLayer().GetMouseScrolled();
}

std::vector<int> pig::Input::GetKeysTyped()
{
	return pig::Application::Get().GetInputLayer().GetKeysTyped();
}
