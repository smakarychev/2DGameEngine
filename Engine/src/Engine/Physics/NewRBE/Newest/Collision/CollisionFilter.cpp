#include "enginepch.h"

#include "Colliders/Collider2D.h"
#include "CollisionFilter.h"

namespace Engine::WIP::Physics::Newest
{
    bool CollisionFilter::ShouldCollide(Collider2D* first, Collider2D* second)
    {
        const CollisionFilter& filterA = first->GetCollisionFilter();
        const CollisionFilter& filterB = second->GetCollisionFilter();
        if (filterA.GroupIndex == 0 || filterA.GroupIndex != filterB.GroupIndex)
        {
            // Use mask + category.
            return (filterA.CategoryBits & filterB.MaskBits) && (filterB.CategoryBits & filterA.MaskBits);
        }
        return filterA.GroupIndex > 0;
    }
}
