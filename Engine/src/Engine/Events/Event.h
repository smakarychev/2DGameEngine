#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Core.h"

namespace Engine
{
	using namespace Types;
	namespace EventCategory
	{
		enum Category
		{
			None = 0,
			Application = Bit(1),
			Input = Bit(2),
			Keyboard = Bit(3),
			Mouse = Bit(4),
			MouseButton = Bit(5)
		};
	}

	namespace EventType
	{
		enum Type
		{
			None = 0,
			// Window events
			WindowClose, WindowResize, WindowFocused, WindowLostFocus, WindowMoved,
			// Keyboard events
			KeyPressed, KeyReleased, KeyTyped,
			// Mouse events
			MouseMoved, MouseButtonPressed, MouseButtonReleased, MouseScrolled
		};
	}
	

#define EVENT_CLASS_TYPE(type) static EventType::Type GetStaticType() { return EventType::type; }\
								virtual EventType::Type GetType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual U32 GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		virtual ~Event() = default;
		bool Handled = false;

		virtual EventType::Type GetType() const = 0;
		virtual U32 GetCategoryFlags() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory::Category category) { return GetCategoryFlags() & category; }
		bool IsOfType(EventType::Type type) { return GetType() == type; }
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