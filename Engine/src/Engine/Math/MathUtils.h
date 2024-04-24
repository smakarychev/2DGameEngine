#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <algorithm>
#include <glm/glm.hpp>

namespace Engine::Math
{
	using namespace Types;
	inline U32 CLZ(U32 number)
	{
		return __lzcnt(number);
#			ifndef _MSC_VER
		return __builtin_clz(number);
#			endif
	}

	inline U64 CLZ(U64 number)
	{
		return __lzcnt64(number);
#			ifndef _MSC_VER
		return __builtin_clz(number);
#			endif
	}

	constexpr bool IsPowerOf2(std::integral auto val)
	{
		return (val & (val - 1)) == 0;
	}
	
	// Return a number which is >= `number` and is a power of 2.
	inline U32 CeilToPower2(U32 number)
	{
		return number == 1 ? 1 : (U32)1 << (32 - CLZ(number - 1));
	}

	// Return a number which is >= `number` and is a power of 2.
	inline U64 CeilToPower2(U64 number)
	{
		return number == 1 ? 1 : (U64)1 << (64 - CLZ(number - 1));
	}

	// Returns the ceil(log2(number)).
	inline U32 Log2(U32 number)
	{
		ENGINE_CORE_ASSERT(number != 0, "Log2 of 0 is undefined.")
		return 31 - CLZ(number);
	}

	// Returns the ceil(log2(number)).
	inline U64 Log2(U64 number)
	{
		ENGINE_CORE_ASSERT(number != 0, "Log2 of 0 is undefined.")
		return 63 - CLZ(number);
	}

	// Returns true if `a` and `b` are almost equal.
	inline bool CompareEqual(F32 a, F32 b)
	{
		return std::abs(a - b) <= std::numeric_limits<F32>::epsilon() * std::max(1.0f, std::max(std::abs(a), std::abs(b)));
	}

	// Returns true if `a` and `b` are almost equal.
	inline bool CompareEqual(F64 a, F64 b)
	{
		return std::abs(a - b) <= std::numeric_limits<F64>::epsilon() * std::max(1.0, std::max(std::abs(a), std::abs(b)));
	}

	// Returns `value mod base` when base is the power of 2.
	constexpr std::integral auto FastMod(std::integral auto value, std::integral auto base)
	{
		ENGINE_CORE_ASSERT(IsPowerOf2(base), "Base have to be a power of 2.")
		return value & (base - 1);
	}

	// Clamps `val` between `min` and `max`.
	template <typename T, typename U, typename V>
	decltype(auto) Clamp(T val, U min, V max)
	{
		return std::clamp<std::common_type_t<T, U, V>>(val, min, max);
	}

	template <typename T>
	T Abs(const T& val)
	{
		return std::abs(val);
	}

	template <>
	inline glm::vec2 Abs<glm::vec2>(const glm::vec2& val)
	{
		return glm::vec2{ Math::Abs(val.x), Math::Abs(val.y) };
	}

	constexpr std::floating_point auto Sqrt(std::floating_point auto val)
	{
		return std::sqrt(val);
	}

	template <typename T, typename U>
	constexpr decltype(auto) Max(const T& first, const U& second)
	{
		return std::max<std::common_type_t<T, U>>(first, second);
	}
	template <typename T>
	constexpr decltype(auto) Max(const glm::vec<2, T>& first, const glm::vec<2, T>& second)
	{
		return glm::vec<2, T>{Max(first.x, second.x), Max(first.y, second.y)};
	}
	template <typename T>
	constexpr decltype(auto) Max(const glm::vec<3, T>& first, const glm::vec<3, T>& second)
	{
		return glm::vec<3, T>{Max(first.x, second.x), Max(first.y, second.y), Max(first.z, second.z)};
	}
	template <typename T>
	constexpr decltype(auto) Max(const glm::vec<4, T>& first, const glm::vec<4, T>& second)
	{
		return glm::vec<4, T>{Max(first.x, second.x), Max(first.y, second.y), Max(first.z, second.z), Max(first.w, second.w)};
	}
	
	template <typename T, typename U>
	constexpr decltype(auto) Min(const T& first, const U& second)
	{
		return std::min<std::common_type_t<T, U>>(first, second);
	}
	template <typename T>
	constexpr decltype(auto) Min(const glm::vec<2, T>& first, const glm::vec<2, T>& second)
	{
		return glm::vec<2, T>{Min(first.x, second.x), Min(first.y, second.y)};
	}
	template <typename T>
	constexpr decltype(auto) Min(const glm::vec<3, T>& first, const glm::vec<3, T>& second)
	{
		return glm::vec<3, T>{Min(first.x, second.x), Min(first.y, second.y), Min(first.z, second.z)};
	}
	template <typename T>
	constexpr decltype(auto) Min(const glm::vec<4, T>& first, const glm::vec<4, T>& second)
	{
		return glm::vec<4, T>{Min(first.x, second.x), Min(first.y, second.y), Min(first.z, second.z), Min(first.w, second.w)};
	}

	template <typename T = float>
	constexpr T Pi()
	{
		return static_cast<T>(3.14159265358979323846);
	}

	constexpr U64 U32PairKey(std::pair<U32, U32> pair)
	{
		return static_cast<U64>(pair.first) << 32 | static_cast<U32>(pair.second);
	}

	constexpr I64 I32PairKey(std::pair<I32, I32> pair)
	{
		return static_cast<I64>(pair.first) << 32 | pair.second;
	}

	constexpr std::floating_point auto SnapToGrid(std::floating_point auto val, std::floating_point auto size)
	{
		return std::floor((val + size * 0.5f) / size) * size;
	}
	
	constexpr std::floating_point auto Align(std::floating_point auto val, std::floating_point auto size)
	{
		return SnapToGrid(val, size);
	}

	constexpr glm::vec2 SnapToGrid(const glm::vec2& val, const glm::vec2& size)
	{
		return { Align(val.x, size.x), Align(val.y, size.y) };
	}

	constexpr glm::vec2 Align(const glm::vec2& val, const glm::vec2& size)
	{
		return SnapToGrid(val, size);
	}

	constexpr std::floating_point auto Lerp(std::floating_point auto a, std::floating_point auto b, std::floating_point auto t)
	{
		return a + (b - a) * t;
	}

	constexpr std::floating_point auto ILerp(std::floating_point auto a, std::floating_point auto b, std::floating_point auto t)
	{
		return (t - a) / (b - a);
	}
}
