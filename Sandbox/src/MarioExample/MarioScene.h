#pragma once

#include "Engine.h"
#include "MarioContactListener.h"
#include "Engine/ECS/Registry.h"

using namespace Engine;
using namespace Engine::Types;

class MarioScene final : public Scene
{
public:
    void OnInit() override;
    void OnUpdate(F32 dt) override;
    void OnEvent(Event& event) override;
    void OnRender() override;
    void OnImguiUpdate() override;
    void PerformAction(Action& action) override;
public:
    // 'Systems'
    void SPhysics(F32 dt);
    // Renderer and Framebuffer are bounded in parenting layer.
    // TODO: toggle between editor/runtime rendering (currently it is editor for mouse picking).
    void SRender();
    void SMove();
    void SAnimation(F32 dt);
    void SState();
private:
    void AddPlayer();
    // TODO: save / load level from file (not a real reflection obv. (for now :^)).
    void CreateLevel();

    bool OnMousePressed(MouseButtonPressedEvent& event);
private:
    MarioContactListener m_ContactListener;
    SortingLayer m_SortingLayer;
    
    // Assets.
    Ref<Texture> m_BrickTexture;
    Ref<Texture> m_MarioSprites;
    Ref<Font> m_Font;
    std::vector<Ref<SpriteAnimation>> m_Animations;
    std::unordered_map<std::string, SpriteAnimation*> m_AnimationsMap;
};
