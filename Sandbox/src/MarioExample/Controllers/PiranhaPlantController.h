#pragma once

#include "Engine.h"
#include "../MarioContactListener.h"

class MarioScene;

class PiranhaPlantController
{
public:
    struct Config
    {
        F32 VerticalSpeed{1.0f};
        F32 Amplitude{1.0f};
        F32 ApexDelay{1.0f};
    };
public:
    PiranhaPlantController(MarioScene& scene);
    void OnUpdate(F32 dt);
    void ReadConfig(const std::string& path);
private:
private:
    MarioScene& m_Scene;
    Config m_Config{};

    CollisionCallback::SensorCallback m_HitCallback{};
};
