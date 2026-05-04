#pragma once

#include "Event.h"

namespace pig {

	class PIGEON_API KeyEvent :
		public EventClassCategory< EventCategoryKeyboard | EventCategoryInput >
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; }

	protected:
		KeyEvent(int keycode)
			: m_KeyCode(keycode) {}

		int m_KeyCode;
	};

	class PIGEON_API KeyPressedEvent : 
		public KeyEvent,
		public EventClassType<EventType::KeyPressed>
	{
	public:
		KeyPressedEvent(int keycode, int repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

	private:
		int m_RepeatCount;
	};

	class PIGEON_API KeyReleasedEvent : 
		public KeyEvent,
		public EventClassType<EventType::KeyReleased>
	{
	public:
		KeyReleasedEvent(int keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}
	};

	class PIGEON_API KeyTypedEvent : 
		public KeyEvent,
		public EventClassType<EventType::KeyTyped>
	{
	public:
		KeyTypedEvent(int keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}
	};
}