#pragma once

#include <cinttypes>

namespace Engine
{
	namespace Types
	{
		using I8 = int8_t;
		using I16 = int16_t;
		using I32 = int32_t;
		using I64 = int64_t;

		using U8 = uint8_t;
		using U16 = uint16_t;
		using U32 = uint32_t;
		using U64 = uint64_t;

		using F32 = float;
		using F64 = double;

		inline auto operator""_B(const U64 val)   -> U64 { return val; }
		inline auto operator""_KiB(const U64 val) -> U64 { return 1024 * val; }
		inline auto operator""_MiB(const U64 val) -> U64 { return 1024 * 1024 * val; }
		inline auto operator""_GiB(const U64 val) -> U64 { return 1024 * 1024 * 1024 * val; }
	}
}

