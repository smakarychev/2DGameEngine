#include "enginepch.h"

#include "Window.h"

#include "Engine/Platform/Windows/WindowsWindow.h"

namespace Engine
{
	std::unique_ptr<Window> Window::Create(const WindowProps& props)
	{
		// Should have #ifdef, etc. to choose what window to return
		return std::make_unique<WindowsWindow>(props);
	}
}