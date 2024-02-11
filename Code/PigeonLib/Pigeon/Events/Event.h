#pragma once

#include "Pigeon/Core.h"

namespace pig {

	// Events in Pigeon are currently blocking, meaning when an event occurs it
	// immediately gets dispatched and must be dealt with right then an there.
	// For the future, a better strategy might be to buffer events in an event
	// bus and process them during the "event" part of the update stage.

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	static const std::unordered_map<EventType, std::string> EventTypeToString = 
	{
		{EventType::None, "None"},
		{EventType::WindowClose, "WindowClose"},
		{EventType::WindowResize, "WindowResize"},
		{EventType::WindowFocus, "WindowFocus"},
		{EventType::WindowLostFocus, "WindowLostFocus"},
		{EventType::WindowMoved, "WindowMoved"},
		{EventType::AppTick, "AppTick"},
		{EventType::AppUpdate, "AppUpdate"},
		{EventType::AppRender, "AppRender"},
		{EventType::KeyPressed, "KeyPressed"},
		{EventType::KeyReleased, "KeyReleased"},
		{EventType::KeyTyped, "KeyTyped"},
		{EventType::MouseButtonPressed, "MouseButtonPressed"},
		{EventType::MouseButtonReleased, "MouseButtonReleased"},
		{EventType::MouseMoved, "MouseMoved"},
		{EventType::MouseScrolled, "MouseScrolled"}
	};

	template<EventType Type>
	class EventClassType : public virtual pig::Event
	{
	public:
		static EventType GetStaticType() { return Type; }
		EventType GetEventType() const override { return GetStaticType(); }
		virtual std::string GetName() const override { return EventTypeToString.at(Type); }
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication    = BIT(0),
		EventCategoryInput          = BIT(1),
		EventCategoryKeyboard       = BIT(2),
		EventCategoryMouse          = BIT(3),
		EventCategoryMouseButton    = BIT(4)
	};

	template<int Category>
	class EventClassCategory : public virtual pig::Event
	{
	public:
		int GetCategoryFlags() const override { return Category; };
	};

	class PIGEON_API Event
	{
	public:
		virtual ~Event() = default;

		virtual EventType GetEventType() const = 0;
		virtual std::string GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }
		virtual int GetCategoryFlags() const { return EventCategory::None; };
		inline bool IsInCategory(EventCategory category) { return GetCategoryFlags() & category; };
	};

	class EventDispatcher 
	{
	public:
		template<typename T>
		static bool Dispatch(const Event& event, std::function<bool(const T&)> func)
		{
			if (event.GetEventType() == T::GetStaticType()) 
			{
				const T* derivedEvent = dynamic_cast<const T*>(&event);
				if (derivedEvent)
				{
					return func(*derivedEvent);
				}
			}
			return false;
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}

