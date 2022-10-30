#pragma once

#include "Application.h"
#include "Log.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Contacts.h"

extern std::unique_ptr<Engine::Application> Engine::createApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();
	Engine::MemoryManager::Init();
	// Scope, so app gets destroyed before MemoryManager.
	{
		auto app = Engine::createApplication();
		app->Run();
	}
	Engine::Renderer::ShutDown();
	Engine::ResourceManager::ShutDown();
	Engine::DefaultContactListener::Shutdown();
	Engine::MemoryManager::ShutDown();
}