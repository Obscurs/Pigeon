#pragma once

#include "Event.h"

namespace pig {

	class PIGEON_API MouseMovedEvent : 
		public EventClassCategory<EventCategoryMouse | EventCategoryInput>,
		public EventClassType<EventType::MouseMoved>
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

		inline float GetX() const { return m_MouseX; }
		inline float GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

	private:
		float m_MouseX, m_MouseY;
	};

	class PIGEON_API MouseScrolledEvent : 
		public EventClassCategory<EventCategoryMouse | EventCategoryInput>,
		public EventClassType<EventType::MouseScrolled>
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		inline float GetXOffset() const { return m_XOffset; }
		inline float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

	private:
		float m_XOffset, m_YOffset;
	};

	class PIGEON_API MouseButtonEvent : 
		public EventClassCategory< EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton >
	{
	public:
		inline int GetMouseButton() const { return m_Button; }

	protected:
		MouseButtonEvent(int button)
			: m_Button(button) {}

		int m_Button;
	};

	class PIGEON_API MouseButtonPressedEvent : 
		public MouseButtonEvent,
		public EventClassType<EventType::MouseButtonPressed>
	{
	public:
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}
	};

	class PIGEON_API MouseButtonReleasedEvent : 
		public MouseButtonEvent, 
		public EventClassType<EventType::MouseButtonReleased>
	{
	public:
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}
	};

}