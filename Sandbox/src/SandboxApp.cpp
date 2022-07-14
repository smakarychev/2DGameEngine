#include <Engine.h>

#include <memory>

class SandboxApp : public Engine::Application
{
public:
	SandboxApp()
	{
		ENGINE_CORE_TRACE("Test message on {} {}", __FILE__, __LINE__);
	}
	~SandboxApp()
	{

	}
};

std::unique_ptr<Engine::Application> Engine::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}
