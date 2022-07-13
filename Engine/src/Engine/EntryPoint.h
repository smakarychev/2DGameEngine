#pragma once

#include <memory>

#include "Application.h"

extern std::unique_ptr<Engine::Application> Engine::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Engine::CreateApplication();
	app->Run();
}