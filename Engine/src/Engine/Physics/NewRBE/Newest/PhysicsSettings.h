#pragma once

#include "Engine/Core/Types.h"

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;

    struct PhysicsSettings
    {
        glm::vec2 GravityVector = { 0.0f, -9.81f };
        
        F32 BaumgarteCoefficient = 0.2f;

        U32 VelocitySteps = 10;

        U32 PositionSteps = 8;

        // Min time before body can go to sleep (seconds).
        F32 MinToSleepTime = 0.5f;

        // Max body vel magnitude for object to be able to go to sleep (meters/second).
        F32 MaxBodySleepVel = 0.02f;

        bool AllowSleep = true;

        bool EnableWarmStart = true;
    };
    
}
