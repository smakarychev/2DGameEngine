#include <Engine.h>
#include <Engine/Core/EntryPoint.h>

#include "GemWarsExample/GemWarsExample.h"
#include "QuadTreeExample/QuadTreeExample.h"
#include "ParticlePhysicsExample/ParticlePhysicsExample.h"
#include "RigidBodyPhysicsExample/RigidBodyPhysicsExample.h"
#include "MarioExample/MarioGame.h"

class SandboxApp : public Engine::Application
{
public:
	SandboxApp()
	{
		//PushLayer(Engine::CreateRef<GemWarsExample>());
		//PushLayer(Engine::CreateRef<QuadTreeExample>());
		//PushLayer(Engine::CreateRef<ParticlePhysicsExample>());
		//PushLayer(Engine::CreateRef<RigidBodyPhysicsExample>());
		PushLayer(Engine::CreateRef<MarioGame>());
	}
	~SandboxApp()
	{

	}
};


std::unique_ptr<Engine::Application> Engine::createApplication()
{
	return std::make_unique<SandboxApp>();
}
