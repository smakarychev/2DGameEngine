#pragma once

#include "Engine.h"
#include "MarioContactListener.h"
#include "MarioTags.h"
#include "Animators/GoombaAnimator.h"
#include "Animators/KoopaAnimator.h"
#include "Animators/PiranhaPlantAnimator.h"
#include "Animators/PlayerAnimator.h"
#include "Controllers/GoombaController.h"
#include "Controllers/KoopaController.h"
#include "Controllers/PiranhaPlantController.h"
#include "Controllers/PlayerController.h"
#include "FSM/PlayerFSM.h"


using namespace Engine;
using namespace Engine::Types;

class MarioScene final : public Scene
{
public:
    MarioScene();

    void Open(const std::string& filename) override;
    void Save(const std::string& filename) override;
    void Clear() override;
    
    void OnInit() override;
    void InitPlayer();
    void InitGoomba();
    void InitPiranhaPlant();
    void InitKoopa();
    void InitCameraController();
    void OnScenePlay() override;
    void OnSceneStop() override;
    void OnUpdate(F32 dt) override;
    void OnEvent(Event& event) override;
    void OnRender() override;
    void OnImguiUpdate() override;
    void OnSceneGlobalUpdate() override;
    void PerformAction(Action& action) override;
    Component::Camera* GetMainCamera() override;
    FrameBuffer* GetMainFrameBuffer() override;

    void AddSensorCallback(const std::string& indexMajor, const std::string& indexMinor, CollisionCallback::SensorCallback callback);
    
public:
    // 'Systems'
    void SPhysics(F32 dt);
    // TODO: toggle between editor/runtime rendering (currently it is editor for mouse picking).
    void SRender();
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
    Ref<Texture> m_MarioSprites;
    Ref<Font> m_Font;
    std::unordered_map<std::string, Ref<SpriteAnimation>> m_AnimationsMap;
    std::unordered_map<std::string, std::unordered_map<std::string, CollisionCallback::SensorCallback>> m_SensorCallbacks;

    std::string m_SceneLoadPath{};

    bool m_IsPlaying{false};

    PlayerController m_PlayerController;
    PlayerAnimator m_PlayerAnimator;

    GoombaController m_GoombaController;
    GoombaAnimator m_GoombaAnimator;

    PiranhaPlantController m_PiranhaPlantController;
    PiranhaPlantAnimator m_PiranhaPlantAnimator;

    KoopaController m_KoopaController;
    KoopaAnimator m_KoopaAnimator;

    PlayerFSM m_PlayerFsm;
    
};
