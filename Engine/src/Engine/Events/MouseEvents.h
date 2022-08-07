#pragma once

#include "Event.h"
#include "Engine/Core/MouseCodes.h"

namespace Engine
{
	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
	protected:
		MouseButtonEvent(MouseCode button)
			: m_Button(button) {}

		MouseCode m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		EVENT_CLASS_TYPE(MouseButtonPressed)

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		EVENT_CLASS_TYPE(MouseButtonReleased)

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}
	};

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(F32 x, F32 y)
			: m_MouseX(x), m_MouseY(y) {}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
		
		F32 GetX() const { return m_MouseX; }
		F32 GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

	private:
		F32 m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(F32 xOffset, F32 yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		F32 GetXOffset() const { return m_XOffset; }
		F32 GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		F32 m_XOffset, m_YOffset;
	};
}