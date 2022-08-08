#include "enginepch.h"

#include "WindowsWindow.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Core.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/KeyboardEvents.h"
#include "Engine/Events/MouseEvents.h"
#include "Engine/Events/WindowEvents.h"

#include "Engine/Memory/MemoryManager.h"

#include "Platform/OpenGL/OpenGLContext.h"

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

		// Initialize glfw.
		ENGINE_CORE_INFO("Initializing the GLFW: version {}", glfwGetVersionString());
		auto status = glfwInit();
		ENGINE_CORE_ASSERT(status != GLFW_FALSE, "Failed to initialize glfw.");


		// Create an glfwwindow.
		ENGINE_CORE_INFO("Creating a window \"{}\" ({} {}).", m_Data.Title, m_Data.Width, m_Data.Height);
		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		ENGINE_CORE_ASSERT(m_Window != 0, "Failed to create glfwWindow.");

		// Create an opengl context.
		m_Context = New<OpenGLContext>(m_Window);
		m_Context->Init();


		// Set glfw error callback.
		glfwSetErrorCallback([](I32 error, const char* description) { ENGINE_CORE_ERROR("GLFW error #{}: {}.", error, description); });
		
		// Set all (needed) event callbacks.
		glfwSetWindowUserPointer(m_Window, &m_Data);
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) 
			{
				WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				WindowCloseEvent event;
				data.EventCallbackFn(event);
			});
		
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, I32 width, I32 height) 
			{
				WindowData data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				data.Width = U32(width);
				data.Height = U32(height);

				WindowResizeEvent event{ U32(width), U32(height) };
				data.EventCallbackFn(event);
			});
		
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallbackFn(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallbackFn(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.EventCallbackFn(event);
					break;
				}
				}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

				KeyTypedEvent event(keycode);
				data.EventCallbackFn(event);
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallbackFn(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallbackFn(event);
					break;
				}
				}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

				MouseScrolledEvent event((F32)xOffset, (F32)yOffset);
				data.EventCallbackFn(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

				MouseMovedEvent event((F32)xPos, (F32)yPos);
				data.EventCallbackFn(event);
			});

		SetVSync(true);
	}
	
	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
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
		Delete<GraphicsContext>(m_Context);
		glfwTerminate();
	}
}