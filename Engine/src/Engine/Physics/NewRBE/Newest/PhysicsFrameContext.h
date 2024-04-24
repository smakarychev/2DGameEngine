#pragma once

#include "BodyPair.h"
#include "Collision/NarrowPhase/Contact.h"
#include "Engine/Common/TwoFrameBuffer.h"
#include "Engine/Core/Types.h"

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;

    using FrameContextContactAllocator = StackAllocator;
    
    struct PhysicsFrameContext
    {
        F32 DeltaTime{0.0f};

        TwoFrameBuffer<std::unordered_map<BodyPairHash, ContactInfo2D>> ContactsCache;

        FrameContextContactAllocator ContactAllocator{2_MiB};
    };
}
