#pragma once

#include "Application.h"
#include "Log.h"
#include "Engine/Memory/MemoryManager.h"

extern std::unique_ptr<Engine::Application> Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();
	Engine::MemoryManager::Init();
	auto app = Engine::CreateApplication();
	app->Run();
	Engine::MemoryManager::ShutDown();
}