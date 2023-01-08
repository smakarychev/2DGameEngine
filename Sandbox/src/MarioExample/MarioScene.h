#pragma once

#include "Engine.h"
#include "MarioContactListener.h"
#include "MarioTags.h"
#include "FSM/BlockFSM.h"

#include "FSM/PlayerFSM.h"
#include "FSM/GoombaFSM.h"
#include "FSM/KoopaFSM.h"
#include "FSM/PiranhaPlantFSM.h"


using namespace Engine;
using namespace Engine::Types;

class MarioScene final : public Scene
{
public:
    enum class GameState
    {
        Menu, Running, GameOver, GameWin // `win` is more popular than `won`.
    };
public:
    MarioScene();

    void Open(const std::string& filename) override;
    void Save(const std::string& filename) override;
    void Clear() override;
    
    void OnInit() override;
    void OnScenePlay() override;
    void OnSceneStop() override;
    void OnUpdate(F32 dt) override;
    void OnEvent(Event& event) override;
    void OnRender() override;
    void OnImguiUpdate() override;
    void OnSceneGlobalUpdate(Entity addedEntity) override;
    Component::Camera* GetMainCamera() override;
    FrameBuffer* GetMainFrameBuffer() override;

    void AddSensorCallback(const std::string& callbackName, CollisionCallback::SensorCallback callback);
    
    void PerformAction(Action& action) override {}
public:
    // 'Systems'
    void SKill(F32 dt);
    void SPhysics(F32 dt);
    // TODO: toggle between editor/runtime rendering (currently it is editor for mouse picking).
    void SRender();
    void SRenderText();
    void SAnimation(F32 dt);
    void SCamera();
    void SGameState();
    void SGameMenu();
private:
    void InitEntities();
    void InitPlayer();
    void InitGoomba();
    void InitPiranhaPlant();
    void InitKoopa();
    void InitBlock();
    void InitCameraController();
    void InitGameWinCollisionCallback();

    void RenderEditor();
    void ValidateViewport();

private:
    MarioContactListener m_ContactListener;
    SortingLayer m_SortingLayer;
    
    // Assets.
    Ref<Texture> m_MarioSprites;
    Ref<Font> m_Font;
    std::unordered_map<std::string, Ref<SpriteAnimation>> m_AnimationsMap;
    std::unordered_map<std::string, CollisionCallback::SensorCallback> m_SensorCallbacks;

    std::string m_SceneLoadPath{};

    bool m_IsPlaying{true};

    PlayerFSM m_PlayerFsm;
    GoombaFSM m_GoombaFsm;
    PiranhaPlantFSM m_PiranhaPlantFsm;
    KoopaFSM m_KoopaFsm;
    BlockFSM m_BlockFsm;

    Entity m_Player{NULL_ENTITY};
    GameState m_GameState{GameState::Menu};
    std::string m_DefaultScene{"Game menu"};
    std::string m_CurrentScene{};
};
