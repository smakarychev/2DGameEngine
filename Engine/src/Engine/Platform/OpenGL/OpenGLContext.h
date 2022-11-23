#pragma once

#include "Engine/Rendering/GraphicsContext.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Engine
{
	using namespace Types;

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}