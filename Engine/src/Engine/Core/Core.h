#pragma once

#include "Log.h"
#include "Engine/Core/Types.h"

#define ENGINE_ASSERT(x, ...) if (x) {} else { ENGINE_ERROR("Assertion failed: {}", __VA_ARGS__); __debugbreak(); }
#define ENGINE_CORE_ASSERT(x, ...) if (x) {} else { ENGINE_CORE_ERROR("Assertion failed: {}", __VA_ARGS__); __debugbreak(); }

#define BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define BIND_FN_STATIC(fn) [](auto&&... args) -> decltype(auto) { return fn(std::forward<decltype(args)>(args)...); }

#define FRIEND_MEMORY_FN	template <typename T, typename ... Args> \
							friend T* Engine::New(Args&&... args); \
							template <typename T, U64 Count, typename ... Args> \
							friend T* Engine::New(Args&&... args); \
							template <typename T, typename ... Args> \
							friend T* Engine::NewArr(U64 count, Args&&... args); \
							template <typename T> \
							friend void Engine::Delete(T * obj); \
							template <typename T, U64 Count> \
							friend void Engine::Delete(T * obj); \
							template <typename T> \
							friend void Engine::DeleteArr(T * obj, U64 count);

constexpr Engine::Types::U64 Bit(Engine::Types::U64 pos) { return Engine::Types::U64(1) << pos; }

namespace Engine
{
	template <typename T, typename ... Args>
	T* New(Args&&... args);
	template <typename T>
	void Delete(T* obj);

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::shared_ptr<T>(New<T>(std::forward<Args>(args)...), Delete<T>);
	}

	template<typename T>
	using Scope = std::unique_ptr<T, decltype(&Delete<T>)>;

	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::unique_ptr<T, decltype(&Delete<T>)>(New<T>(std::forward<Args>(args)...), Delete<T>);
	}
}

