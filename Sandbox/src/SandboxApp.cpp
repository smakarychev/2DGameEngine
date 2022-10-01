#include <Engine.h>
#include <Engine/Core/EntryPoint.h>

#include "GemWarsExample/GemWarsExample.h"
#include "QuadTreeExample/QuadTreeExample.h"
#include "ParticlePhysicsExample/ParticlePhysicsExample.h"
#include "RigidBodyPhysicsExample/RigidBodyPhysicsExample.h"

#include <chrono> // TEMP
#include <random>


class SandboxApp : public Engine::Application
{
public:
	SandboxApp()
	{
		//PushLayer(Engine::CreateRef<GemWarsExample>());
		//PushLayer(Engine::CreateRef<QuadTreeExample>());
		//PushLayer(Engine::CreateRef<ParticlePhysicsExample>());
		PushLayer(Engine::CreateRef<RigidBodyPhysicsExample>());
	}
	~SandboxApp()
	{

	}
};


std::unique_ptr<Engine::Application> Engine::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}
