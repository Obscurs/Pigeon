#pragma once

#include "Event.h"

namespace pig 
{
	class PIGEON_API WindowResizeEvent : 
		public EventClassCategory<EventCategoryApplication>,
		public EventClassType<EventType::WindowResize>
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {}

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

	private:
		unsigned int m_Width, m_Height;
	};

	class PIGEON_API WindowCloseEvent : 
		public EventClassType<EventType::WindowClose>,
		public EventClassCategory<EventCategoryApplication>
	{
	public:
		WindowCloseEvent() {}
	};
}