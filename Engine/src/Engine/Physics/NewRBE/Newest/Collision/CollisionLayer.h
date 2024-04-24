#pragma once
#include "Engine/Core/Types.h"

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;

    using CollisionLayer = U32;
    static constexpr auto CL_INVALID_LAYER = std::numeric_limits<U32>::max();

    class BroadPhaseLayers
    {
    public:
        virtual ~BroadPhaseLayers() = default;
        // Broad phase needs to know, how many trees to create.
        virtual U32 GetLayersCount() const = 0;
    };
    
    class BodyToBroadPhaseLayerFilter
    {
    public:
        virtual ~BodyToBroadPhaseLayerFilter() = default;
        virtual bool ShouldCollide(CollisionLayer bodyLayer, CollisionLayer bPhaseLayer) = 0;
    };
    
}

