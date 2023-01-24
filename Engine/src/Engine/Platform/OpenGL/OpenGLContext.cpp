#include "enginepch.h"

#include "Engine/Core/Core.h"

#include "OpenGLContext.h"


namespace Engine
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
	{
		ENGINE_CORE_ASSERT(m_WindowHandle, "Window handle is nullptr.")
	}
	void OpenGLContext::Init()
	{
		// Attach opengl to window.
		glfwMakeContextCurrent(m_WindowHandle);
		auto status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ENGINE_CORE_ASSERT(status != 0, "Failed to initialize glad.")
	}
	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}