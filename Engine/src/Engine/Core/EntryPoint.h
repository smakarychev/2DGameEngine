#pragma once

#include "Application.h"
#include "Log.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Resource/ResourceManager.h"

extern std::unique_ptr<Engine::Application> Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();
	Engine::MemoryManager::Init();
	// Scope, so app gets destroyed before MemoryManager.
	{
		auto app = Engine::CreateApplication();
		app->Run();
	}
	Engine::ShaderLoader::ShutDown();
	Engine::MemoryManager::ShutDown();
}