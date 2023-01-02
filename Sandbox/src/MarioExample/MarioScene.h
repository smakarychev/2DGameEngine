#pragma once

#include "Engine.h"
#include "MarioContactListener.h"
#include "Engine/ECS/Registry.h"

using namespace Engine;
using namespace Engine::Types;

class MarioScene final : public Scene
{
public:
    void Open(const std::string& filename) override;
    void Save(const std::string& filename) override;
    void Clear() override;
    
    void OnInit() override;
    void OnUpdate(F32 dt) override;
    void OnEvent(Event& event) override;
    void OnRender() override;
    void OnImguiUpdate() override;
    void OnSceneGlobalUpdate() override;
    void PerformAction(Action& action) override;
    Component::Camera* GetMainCamera() override;
    FrameBuffer* GetMainFrameBuffer() override;
public:
    // 'Systems'
    void SPhysics(F32 dt);
    // Renderer and Framebuffer are bounded in parenting layer.
    // TODO: toggle between editor/runtime rendering (currently it is editor for mouse picking).
    void SRender();
    void SMove();
    void SAnimation(F32 dt);
    void SState();
    void SCamera();
private:
    void InitSensorCallbacks();
    // TODO: save / load level from file (not a real reflection obv. (for now :^)).
    void ValidateViewport();
    
    bool OnMousePressed(MouseButtonPressedEvent& event);
private:
    MarioContactListener m_ContactListener;
    SortingLayer m_SortingLayer;
    
    // Assets.
    Ref<Texture> m_BrickTexture;
    Ref<Texture> m_MarioSprites;
    Ref<Font> m_Font;
    std::unordered_map<std::string, Ref<SpriteAnimation>> m_AnimationsMap;
    std::vector<Component::CollisionCallback::SensorCallback> m_SensorCallbacks;

    bool m_RequiresLoad{false};
    std::string m_SceneLoadPath{};
    
};
