#pragma once

#include "Engine/Core/Input.h"
#include "Engine/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Engine
{
	using namespace Types;
	bool Input::GetKey(KeyCode keycode)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, static_cast<I32>(keycode));
		return state == GLFW_PRESS;
	}

	bool Input::GetMouseButton(MouseCode mousecode)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<I32>(mousecode));
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::MousePosition()
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		F64 xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (F32)xpos + s_MainViewportOffset.x, (F32)ypos + s_MainViewportOffset.y };
	}
}