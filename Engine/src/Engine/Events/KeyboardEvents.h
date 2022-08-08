#pragma once

#include "Event.h"
#include "Engine/Core/KeyCodes.h"

namespace Engine
{
	class KeyEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Keyboard)
		
		KeyCode GetKeyCode() const { return m_KeyCode; }
	protected:
		KeyEvent(KeyCode keyCode) : m_KeyCode(keyCode) 
		{}
		KeyCode m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode keyCode, bool isRepeat = false) : KeyEvent(keyCode), m_IsRepeat(isRepeat)
		{}

		EVENT_CLASS_TYPE(KeyPressed)

		bool IsRepeat() const { return m_IsRepeat; }

		std::string ToString() const override 
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " repeat: " <<  m_IsRepeat;
			return ss.str();
		}

	private:
		bool m_IsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode keyCode) : KeyEvent(keyCode)
		{}

		EVENT_CLASS_TYPE(KeyReleased)

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode keyCode) : KeyEvent(keyCode)
		{}

		EVENT_CLASS_TYPE(KeyTyped)

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}
	};
}