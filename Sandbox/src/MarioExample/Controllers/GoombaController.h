#pragma once

#include "Engine.h"
#include "../MarioContactListener.h"

class MarioScene;

class GoombaController
{
public:
    struct Config
    {
        F32 HorizontalSpeed{10.0f};
        F64 DeathTime{1.0};
    };
public:
    GoombaController(MarioScene& scene);
    void OnInit();
    void OnUpdate(F32 dt);
    void ReadConfig(const std::string& path);
private:
    void UpdateState();
    void UpdatePosition(F32 dt);
    void OnGoombaKill(Entity goomba);
private:
    MarioScene& m_Scene;
    Config m_Config{};

    CollisionCallback::SensorCallback m_HitCallback{};
};
