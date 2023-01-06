#pragma once

#include "Engine.h"
#include "../MarioContactListener.h"

class MarioScene;

class KoopaController
{
public:
    struct Config
    {
        F32 HorizontalSpeed{1.0f};
        F32 TurnedHorizontalSpeed{10};
        F64 DeathTime{1.0};
        F32 DeathImpulse{1.0f};
    };
public:
    KoopaController(MarioScene& scene);
    void OnInit();
    void OnUpdate(F32 dt);
    void ReadConfig(const std::string& path);
private:
    void UpdateState();
    void UpdatePosition(F32 dt);
    void OnKoopaKill(Entity koopa);
private:
    MarioScene& m_Scene;
    Config m_Config{};

    CollisionCallback::SensorCallback m_HitCallback{};
};
