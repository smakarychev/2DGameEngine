#pragma once

#include "Engine/Core/Types.h"

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;
    
    class Collider2D;
    struct CollisionFilter
    {
        U16 CategoryBits{0x0001};
        // Which categories to collide with.
        U16 MaskBits{0xFFFF};
        /* 
        Alternative to category + mask, if for a pair of colliders group is:
            * 0 / different - use category + mask.
            * same and negative - never collide.
            * same and positive - always collide.
        */
        I32 GroupIndex{0};
        static bool ShouldCollide(Collider2D* first, Collider2D* second);
    };
}
