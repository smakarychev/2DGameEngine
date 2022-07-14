#include <Engine.h>

#include <memory>

/* ****** Temp section ********** */
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
/**********************************/

class SandboxApp : public Engine::Application
{
public:
	SandboxApp()
	{
		// Everything below me is temp
		ENGINE_CORE_TRACE("Test message on {} {}", __FILE__, __LINE__);
		glfwInit();
	}
	~SandboxApp()
	{

	}
};

std::unique_ptr<Engine::Application> Engine::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}
