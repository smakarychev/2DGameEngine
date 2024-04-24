#pragma once

#include "Log.h"
#include "Engine/Core/Types.h"

#define ENGINE_ASSERT(x, ...) if (x) {} else { ENGINE_ERROR("Assertion failed: {}", __VA_ARGS__); __debugbreak(); }
#define ENGINE_CORE_ASSERT(x, ...) if (x) {} else { ENGINE_CORE_ERROR("Assertion failed: {}", __VA_ARGS__); __debugbreak(); }

#define ENGINE_CHECK_RETURN(x, ...) if (x) {} else { ENGINE_ERROR("{}", __VA_ARGS__); return; }
#define ENGINE_CORE_CHECK_RETURN(x, ...) if (x) {} else { ENGINE_CORE_ERROR("{}", __VA_ARGS__); return; }
#define ENGINE_CHECK_RETURN_NULL(x, ...) if (x) {} else { ENGINE_ERROR("{}", __VA_ARGS__); return nullptr; }
#define ENGINE_CORE_CHECK_RETURN_NULL(x, ...) if (x) {} else { ENGINE_CORE_ERROR("{}", __VA_ARGS__); return nullptr; }

#define BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define BIND_FN_STATIC(fn) [](auto&&... args) -> decltype(auto) { return fn(std::forward<decltype(args)>(args)...); }

#define FRIEND_MEMORY_FN	template <typename T, typename ... Args> \
							friend T* ::Engine::New(Args&&... args); \
							template <typename T, U64 Count, typename ... Args> \
							friend T* ::Engine::New(Args&&... args); \
							template <typename T, typename ... Args> \
							friend T* ::Engine::NewArr(U64 count, Args&&... args); \
							template <typename T, typename Alloc, typename ... Args> \
							friend T* ::Engine::NewAlloc(Alloc& alloc, Args&&... args); \
							template <typename T> \
							friend void ::Engine::Delete(T * obj); \
							template <typename T, U64 Count> \
							friend void ::Engine::Delete(T * obj); \
							template <typename T> \
							friend void ::Engine::DeleteArr(T * obj, U64 count); \
							template <typename T, typename Alloc> \
							friend void ::Engine::DeleteAlloc(Alloc& alloc, T* obj);

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
	class Scope
	{
	public:
		constexpr Scope() : m_Ptr(New<T>(), Delete) {}
		template <typename ... Args>
		constexpr Scope(Args&& ... args) : m_Ptr(New<T>(std::forward<Args>(args)...), Delete) {}
		T* Get() { return m_Ptr.get(); }
		T* Get() const { return m_Ptr.get(); }

		bool operator==(T* ptr) { return m_Ptr == ptr; }
		bool operator!=(T* ptr) { return m_Ptr != ptr; }

		T* operator->() { return Get(); }
		T* operator->() const { return Get(); }
	private:
		std::unique_ptr<T, decltype(&Delete<T>)> m_Ptr;
	};

	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return Scope<T>(std::forward<Args>(args)...);
	}
}

