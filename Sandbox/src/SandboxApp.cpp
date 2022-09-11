#include <Engine.h>
#include <Engine/Core/EntryPoint.h>

#include "GemWarsExample/GemWarsExample.h"
#include "QuadTreeExample/QuadTreeExample.h"
#include "ParticlePhysicsExample/ParticlePhysicsExample.h"

#include <chrono> // TEMP
#include <random>


class SandboxApp : public Engine::Application
{
public:
	SandboxApp()
	{
		//PushLayer(Engine::New<GemWarsExample>());
		//PushLayer(Engine::New<QuadTreeExample>());
		PushLayer(Engine::New<ParticlePhysicsExample>());
	}
	~SandboxApp()
	{

	}
};


std::unique_ptr<Engine::Application> Engine::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}
