#pragma once

#include "Engine/Core/Time.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	using namespace Types;
	F64 Time::Get()
	{
		return glfwGetTime();
	}
}
