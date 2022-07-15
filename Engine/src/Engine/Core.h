#pragma once

#include "Log.h"

#define ENGINE_ASSERT(x, ...) if (x) {} else { ENGINE_ERROR("Assertion failed: {}", __VA_ARGS__); __debugbreak(); }
#define ENGINE_CORE_ASSERT(x, ...) if (x) {} else { ENGINE_CORE_ERROR("Assertion failed: {}", __VA_ARGS__); __debugbreak(); }