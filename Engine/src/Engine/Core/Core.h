#pragma once

#include "Log.h"
#include "Engine/Core/Types.h"

#define ENGINE_ASSERT(x, ...) if (x) {} else { ENGINE_ERROR("Assertion failed: {}", __VA_ARGS__); __debugbreak(); }
#define ENGINE_CORE_ASSERT(x, ...) if (x) {} else { ENGINE_CORE_ERROR("Assertion failed: {}", __VA_ARGS__); __debugbreak(); }

#define BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define BIND_FN_STATIC(fn) [](auto&&... args) -> decltype(auto) { return fn(std::forward<decltype(args)>(args)...); }

constexpr Engine::Types::U64 Bit(Engine::Types::U64 pos) { return Engine::Types::U64(1) << pos; }