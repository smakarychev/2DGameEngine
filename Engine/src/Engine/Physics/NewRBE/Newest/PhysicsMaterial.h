#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/MathUtils.h"

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;

    // Represents the physical properties of a collider, dictates behaviour during the collision resolution. 
    struct PhysicsMaterial
    {
        F32 Friction;
        F32 Restitution;
        F32 Density;
        PhysicsMaterial(F32 friction = 0.1f, F32 restitution = 0.0f, F32 density = 1.0f)
            : Friction(friction), Restitution(restitution), Density(density)
        {}
        static F32 CombineFriction(const PhysicsMaterial& first, const PhysicsMaterial& second)
        {
            return Math::Sqrt(first.Friction * second.Friction);
        }
        static F32 CombineRestitution(const PhysicsMaterial& first, const PhysicsMaterial& second)
        {
            return Math::Max(first.Restitution, second.Restitution);
        }
    };
}
