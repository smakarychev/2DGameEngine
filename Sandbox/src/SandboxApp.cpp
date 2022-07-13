#include <Engine.h>

#include <memory>

class SandboxApp : public Engine::Application
{
public:
	SandboxApp()
	{

	}
	~SandboxApp()
	{

	}
};

std::unique_ptr<Engine::Application> Engine::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}
