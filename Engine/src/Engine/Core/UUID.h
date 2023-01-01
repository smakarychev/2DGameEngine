#pragma once

#include "Types.h"
#include "Engine/Math/Random.h"

namespace Engine
{
    using namespace Types;

    struct UUID
    {
        U64 Id{};
        static UUID Generate() { UUID uuid; uuid.Id = Random::UInt64(); return uuid; }
        operator U64() { return Id; }
        bool operator==(const UUID& other) const { return Id == other.Id; }
        bool operator!=(const UUID& other) const { return !(*this == other); }
    };
}

namespace std
{
    template<>
    struct hash<Engine::UUID>
    {
        size_t operator()(const Engine::UUID& uuid) const noexcept
        {
            return hash<Engine::U64>()(uuid.Id);
        }
    };
    
}
