#include <Engine.h>
#include <Engine/Core/EntryPoint.h>

#include "GemWarsExample.h"

#include <chrono> // TEMP
#include <random>
/* Entry point */


class SandboxApp : public Engine::Application
{
public:
	SandboxApp()
	{
		PushLayer(Engine::New<GemWarsExample>());
	}
	~SandboxApp()
	{

	}
};


std::unique_ptr<Engine::Application> Engine::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}
