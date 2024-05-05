#include "pch.h"
#include "InputLayer.h"

#include <imgui.h>

#include "Pigeon/Core/Application.h"
#include "Pigeon/Core/Input.h"
#include "Pigeon/Core/KeyCodes.h"

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
	//TODO Arnau: typed key codes are incomplete, we do not have codes for both up-down case
	//pig::EventDispatcher::Dispatch<pig::KeyTypedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendKeyEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::MouseMovedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendMouseMoveEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::MouseButtonPressedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendMouseButtonEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::MouseButtonReleasedEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendMouseButtonEvent, pig::InputLayer>(this));
	pig::EventDispatcher::Dispatch<pig::MouseScrolledEvent>(e, pig::BindEventFn<&pig::InputLayer::AppendMouseButtonEvent, pig::InputLayer>(this));

	return false;
}

void pig::InputLayer::OnUpdate(const Timestep& ts)
{
	ProcessEvents();
}

bool pig::InputLayer::IsKeyPressed(int keycode, bool justPressed) const
{
	auto it = m_KeysPressed.find(keycode);
	if (it == m_KeysPressed.end())
		return false;

	return justPressed ? it->second == 1 : true;
}

bool pig::InputLayer::IsKeyReleased(int keycode) const
{
	return m_KeysReleased.find(keycode) != m_KeysReleased.end();
}

bool pig::InputLayer::IsMouseButtonPressed(int button, bool justPressed) const
{
	auto it = m_KeysPressed.find(button);
	if (it == m_KeysPressed.end())
		return false;

	return justPressed ? it->second == 1 : true;
}

bool pig::InputLayer::IsMouseButtonReleased(int button) const
{
	return m_KeysReleased.find(button) != m_KeysReleased.end();
}

glm::vec2 pig::InputLayer::GetMousePosition() const
{
	return m_MousePos;
}

glm::vec2 pig::InputLayer::GetMouseScrolled() const
{
	return m_MouseScroll;
}

bool pig::InputLayer::AppendKeyEvent(const Event& e)
{
	InputEvent inputEvent;
	inputEvent.m_Type = e.GetEventType();

	switch (inputEvent.m_Type)
	{
	case pig::EventType::KeyPressed:
	case pig::EventType::KeyReleased:
	case pig::EventType::KeyTyped:
		inputEvent.m_KeyCode = pig::PlatformInput::GetKeyCode(dynamic_cast<const pig::KeyEvent&>(e).GetKeyCode());
		break;
	default:
		PG_CORE_ASSERT(false, "event type not implemented");
	}
	m_Events.push_back(std::move(inputEvent));
	return false;
}

bool pig::InputLayer::AppendMouseMoveEvent(const Event& e)
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

bool pig::InputLayer::AppendMouseButtonEvent(const Event& e)
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

void pig::InputLayer::ProcessEvents()
{
	m_KeysReleased.clear();
	for (auto it = m_KeysPressed.begin(); it != m_KeysPressed.end(); ++it)
	{
		++it->second;
	}
	for (int i = 0; i < m_Events.size(); ++i)
	{
		const InputEvent& inputEvent = m_Events[i];
		if (inputEvent.m_Type == pig::EventType::KeyPressed || inputEvent.m_Type == pig::EventType::KeyTyped || inputEvent.m_Type == pig::EventType::MouseButtonPressed)
		{
			m_KeysPressed[inputEvent.m_KeyCode] = 1;
		}
		else if (inputEvent.m_Type == pig::EventType::KeyReleased || inputEvent.m_Type == pig::EventType::MouseButtonReleased)
		{
			auto it = m_KeysPressed.find(inputEvent.m_KeyCode);
			if (it != m_KeysPressed.end())
			{
				m_KeysReleased[inputEvent.m_KeyCode] = it->second;
				m_KeysPressed.erase(inputEvent.m_KeyCode);
			}
			else
			{
				m_KeysReleased[inputEvent.m_KeyCode] = 0;
			}
		}
		else if (inputEvent.m_Type == pig::EventType::MouseMoved)
		{
			m_MousePos.x = inputEvent.m_FloatData1;
			m_MousePos.y = inputEvent.m_FloatData2;
		}
		else if (inputEvent.m_Type == pig::EventType::MouseScrolled)
		{
			m_MouseScroll.x = inputEvent.m_FloatData1;
			m_MouseScroll.y = inputEvent.m_FloatData2;
		}
	}
	m_Events.clear();
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
