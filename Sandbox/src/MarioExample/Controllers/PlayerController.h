#pragma once

#include "Engine.h"
#include "../MarioContactListener.h"

using namespace Engine::Types;
using namespace Engine;

class MarioScene;

class PlayerController
{
public:
    struct Config
    {
        F32 MaxHorizontalSpeed{10.0f};
        F32 MaxFallSpeed{-10.0f};
        F32 JumpImpulse{20.0f};
        F32 VerticalVelocityDrop{0.5f};
        F32 HorizontalVelocityDrop{0.5f};
        F64 CoyoteTime{0.5};
        F64 JumpBuffer{0.5};
        F64 DeathTime{1.0};
    };
public:
    PlayerController(MarioScene& scene);
    void OnUpdate(F32 dt);
    void ReadConfig(const std::string& path);
private:
    void GatherInput();
    void UpdatePosition(F32 dt);
    void OnPlayerKill(Entity player);
private:
    MarioScene& m_Scene;
    Config m_Config{};
    F64 m_CoyoteTimeCounter{0.0};
    F64 m_JumpBufferTimeCounter{0.0};
    
    CollisionCallback::SensorCallback m_FootCallback{};
    
};
