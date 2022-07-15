#pragma once

#include "Application.h"
#include "Log.h"

extern std::unique_ptr<Engine::Application> Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Log::Init();
	auto app = Engine::CreateApplication();
	app->Run();
}