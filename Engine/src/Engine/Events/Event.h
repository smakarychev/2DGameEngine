#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Core.h"

namespace Engine
{
	enum EventCategory
	{
		EventCategoryNone = 0,
		EventCategoryApplication = Bit(1),
		EventCategoryInput = Bit(2),
		EventCategoryKeyboard = Bit(3),
		EventCategoryMouse = Bit(4), 
		EventCategoryMouseButton = Bit(5)
	};

	enum EventType
	{
		None = 0,
		// Window events
		WindowClose, WindowResize, WindowFocused, WindowLostFocus, WindowMoved,
		// Keyboard events
		KeyPressed, KeyReleased, KeyTyped,
		// Mouse events
		MouseMoved, MouseButtonPressed, MouseButtonReleased, MouseScrolled
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual U32 GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		virtual ~Event() = default;
		bool Handled = false;

		virtual EventType GetType() const = 0;
		virtual U32 GetCategoryFlags() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category) { return GetCategoryFlags() & category; }
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event) : m_Event(event)
		{}

		// F will be deduced by the compiler
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetType() == T::GetStaticType())
			{
				m_Event.Handled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};
}