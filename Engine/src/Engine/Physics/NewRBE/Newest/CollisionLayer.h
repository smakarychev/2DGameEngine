#pragma once
#include "Engine/Core/Types.h"

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;

    using CollisionLayer = U32;
    static constexpr auto CL_INVALID_LAYER = std::numeric_limits<U32>::max();

    class BodyToBodyLayerFilter
    {
    public:
        virtual ~BodyToBodyLayerFilter();
        virtual bool ShouldCollide(CollisionLayer layerA, CollisionLayer layerB) = 0;
    };

    // Broad phase needs the way to dispatch body to corresponding layer.
    class BroadPhaseLayers
    {
    public:
        virtual ~BroadPhaseLayers();
        virtual CollisionLayer DispatchBodyLayer(CollisionLayer bodyLayer) = 0;
    };
    
    class BodyToBroadPhaseLayerFilter
    {
    public:
        virtual ~BodyToBroadPhaseLayerFilter();
        virtual bool ShouldCollide(CollisionLayer bodyLayer, CollisionLayer bPhaseLayer) = 0;
    };
    
}

