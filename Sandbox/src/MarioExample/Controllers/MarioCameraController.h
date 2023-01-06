#pragma once

#include "Engine.h"

using namespace Engine::Types;
using namespace Engine;

class MarioCameraController final : public CameraController
{
public:
    struct Config
    {
        F32 FollowSpeed{1.0f};
        F32 DistanceThresholdX{0.5f};
        F32 DistanceThresholdY{0.5f};
        glm::vec2 TargetOffset{0.0f, 0.0f};
    };
public:
    MarioCameraController(Ref<Camera> camera, Scene& scene);
    void ReadConfig(const std::string& path);
    void SetTarget(Entity target);
    void OnUpdate(F32 dt) override;
    bool OnEvent(Event& event) override;
private:
    Config m_Config{};
    Entity m_Target{NULL_ENTITY};
    Scene& m_Scene;
};
