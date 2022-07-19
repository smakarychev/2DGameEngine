#include "enginepch.h"

#include "WindowsWindow.h"
#include "Engine/Log.h"
#include "Engine/Core.h"

namespace Engine
{
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		ENGINE_CORE_INFO("Creating a window \"{}\" ({} {}).", m_Data.Title, m_Data.Width, m_Data.Height);

		// Initialize glfw
		auto status = glfwInit();
		ENGINE_CORE_ASSERT(status != GLFW_FALSE, "Failed to initialize glfw.");

		// Create an glfwwindow
		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		ENGINE_CORE_ASSERT(m_Window != 0, "Failed to create glfwWindow.");

		// Attach opengl to window
		glfwMakeContextCurrent(m_Window);

		// Initialize glad (have to be after glfwMakeContextCurrent)
		status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ENGINE_CORE_ASSERT(status != 0, "Failed to initialize glad.");

		// Set glfw error callback
		glfwSetErrorCallback([](I32 error, const char* description) { ENGINE_CORE_ERROR("GLFW error #{}: {}.", error, description); });

		SetVSync(true);
	}
	
	void WindowsWindow::OnUpdate()
	{
		while (!glfwWindowShouldClose(m_Window))
		{
			glClearColor(0.1f, 0.3f, 0.4f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glfwPollEvents();
			glfwSwapBuffers(m_Window);
		}
	}
	
	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
		m_Data.VSync = enabled;
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
}