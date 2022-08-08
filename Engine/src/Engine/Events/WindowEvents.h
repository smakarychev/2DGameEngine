#pragma once

#include "Event.h"

namespace Engine
{
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;
		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategory::Application)
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(U32 newWidth, U32 newHeight) : m_Width(newWidth), m_Height(newHeight)
		{}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategory::Application)

		U32 GetWidth() const { return m_Width; }
		U32 GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

	private:
		U32 m_Width, m_Height;
	};
}