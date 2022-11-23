#include "enginepch.h"

#include "Engine/Platform/Windows/WindowsInput.h"

namespace Engine
{
	std::map<KeyCode, bool>		Input::s_PressedKeys;
	std::map<MouseCode, bool>	Input::s_PressedButtons;
	std::map<KeyCode, bool>		Input::s_ReleasedKeys;
	std::map<MouseCode, bool>	Input::s_ReleasedButtons;
	glm::vec2					Input::s_MainViewportOffset = glm::vec2(0.0f, 0.0f);
	glm::vec2					Input::s_MainViewportSize = glm::vec2(1600.0f, 900.0f);

	void Input::OnUpdate()
	{
		s_PressedKeys.clear();
		s_PressedButtons.clear();
		s_ReleasedKeys.clear();
		s_ReleasedButtons.clear();
	}

	void Input::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<KeyPressedEvent>(BIND_FN_STATIC(Input::SetKeyPressed));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_FN_STATIC(Input::SetKeyReleased));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_FN_STATIC(Input::SetButtonPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_FN_STATIC(Input::SetButtonReleased));
	}


	bool Input::GetKeyDown(KeyCode keycode)
	{
		auto it = s_PressedKeys.find(keycode);
		if (it == s_PressedKeys.end())
		{
			return false;
		}
		else
		{
			return it->second;
		}
	}

	bool Input::GetKeyUp(KeyCode keycode)
	{
		auto it = s_ReleasedKeys.find(keycode);
		if (it == s_ReleasedKeys.end())
		{
			return false;
		}
		else
		{
			return it->second;
		}
	}

	bool Input::GetMouseButtonDown(MouseCode mousecode)
	{
		auto it = s_PressedButtons.find(mousecode);
		if (it == s_PressedButtons.end())
		{
			return false;
		}
		else
		{
			return it->second;
		}
	}
	bool Input::GetMouseButtonUp(MouseCode mousecode)
	{
		auto it = s_ReleasedButtons.find(mousecode);
		if (it == s_ReleasedButtons.end())
		{
			return false;
		}
		else
		{
			return it->second;
		}
	}

	bool Input::SetKeyPressed(KeyPressedEvent& event)
	{
		if (event.IsRepeat()) return false;
		auto keycode = event.GetKeyCode();
		s_PressedKeys[keycode] = true;
		// Returns false to propagate event.
		return false;
	}

	bool Input::SetKeyReleased(KeyReleasedEvent& event)
	{
		auto keycode = event.GetKeyCode();
		s_ReleasedKeys[keycode] = true;
		// Returns false to propagate event.
		return false;
	}

	bool Input::SetButtonPressed(MouseButtonPressedEvent& event)
	{
		auto mouseButton = event.GetMouseButton();
		s_PressedButtons[mouseButton] = true;
		// Returns false to propagate event.
		return false;
	}

	bool Input::SetButtonReleased(MouseButtonReleasedEvent& event)
	{
		auto mouseButton = event.GetMouseButton();
		s_ReleasedButtons[mouseButton] = true;
		// Returns false to propagate event.
		return false;
	}

}