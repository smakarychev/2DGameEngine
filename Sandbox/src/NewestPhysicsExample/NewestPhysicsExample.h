#pragma once

#include <Engine.h>

using namespace Engine::Types;
using namespace Engine;

enum CLayers { Moving = 0, NonMoving = 1 };
class CustomBroadPhaseLayers : public WIP::Physics::Newest::BroadPhaseLayers
{
public:
    U32 GetLayersCount() const override { return static_cast<U32>(m_Layers.size()); }
private:
    std::array<WIP::Physics::Newest::CollisionLayer, 2> m_Layers{ Moving, NonMoving };
};

class CustomBodyToBroadPhaseLayerFilter : public WIP::Physics::Newest::BodyToBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(WIP::Physics::Newest::CollisionLayer bodyLayer, WIP::Physics::Newest::CollisionLayer bPhaseLayer) override
    {
        if (bodyLayer == NonMoving) return bPhaseLayer == Moving;
        return true;
    }
};

class NewestPhysicsExample : public Layer
{
public:
    NewestPhysicsExample();
    void OnAttach() override;
    void OnUpdate() override;
    void OnImguiUpdate() override;
    void OnDetach() override;
private:
    void ValidateViewport();
    void Render();
private:
    WIP::Physics::Newest::PhysicsSystem m_PhysicsSystem;

    
    Ref<CameraController> m_CameraController;
    Ref<Font> m_Font;
    Ref<FrameBuffer> m_FrameBuffer;
    glm::vec2 m_ViewportSize = glm::vec2{ 0.0f };
};
