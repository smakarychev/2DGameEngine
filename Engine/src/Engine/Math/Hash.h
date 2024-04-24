#pragma once

#include "Engine/Core/Types.h"

namespace Engine::Math
{
    using namespace Types;

    static constexpr auto FNV_OFFSET_BASIS = 0xcbf29ce484222325ull;
    static constexpr auto FNV_PRIME = 0x100000001b3ull;
    
    inline U64 HashBytes(const void* data, U32 sizeBytes, U64 offsetBasis = FNV_OFFSET_BASIS)
    {
        U64 hash = offsetBasis;
        for (const U8* byte = static_cast<const U8*>(data); byte < static_cast<const U8*>(data) + sizeBytes; byte++)
        {
            hash = hash ^ static_cast<U64>(*byte);
            hash = hash * FNV_PRIME;
        }
        return hash;
    }
}