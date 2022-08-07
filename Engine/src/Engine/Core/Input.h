#pragma once

#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseCodes.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvents.h"
#include "Engine/Events/KeyboardEvents.h"
#include "Engine/Events/MouseEvents.h"

// TODO: wrapper of glm?
#include <glm/glm.hpp>

#include <map>

namespace Engine
{
	class Input
	{
		// Some (polling) methods shall be defined in platform specific way.
	public:
		static bool GetKey(KeyCode keycode);
		// If key is pressed, but not held (event based).
		static bool GetKeyDown(KeyCode keycode);
		static bool GetKeyUp(KeyCode keycode);

		static bool GetMouseButton(MouseCode mousecode);
		// If button is pressed, but not held (event based).
		static bool GetMouseButtonDown(MouseCode mousecode);
		static bool GetMouseButtonUp(MouseCode mousecode);

		static glm::vec2 MousePosition();

		static void OnUpdate();
		static void OnEvent(Event& event);

	private:
		static std::map<KeyCode, bool> s_PressedKeys;
		static std::map<MouseCode, bool> s_PressedButtons;
		static std::map<KeyCode, bool> s_ReleasedKeys;
		static std::map<MouseCode, bool> s_ReleasedButtons;

		static bool SetKeyPressed(KeyPressedEvent& event);
		static bool SetKeyReleased(KeyReleasedEvent& event);
		static bool SetButtonPressed(MouseButtonPressedEvent& event);
		static bool SetButtonReleased(MouseButtonReleasedEvent& event);

	};
}