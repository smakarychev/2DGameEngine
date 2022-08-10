#include <Engine.h>
#include <Engine/Core/EntryPoint.h>

#include "GameLayer.h"

#include <chrono> // TEMP
#include <random>
/* Entry point */


class SandboxApp : public Engine::Application
{
public:
	SandboxApp()
	{
		GameLayer* layer = Engine::New<GameLayer>();
		PushLayer(layer);
	}
	~SandboxApp()
	{

	}
};


std::unique_ptr<Engine::Application> Engine::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}
