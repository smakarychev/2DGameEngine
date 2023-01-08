#pragma once

#include "FSM.h"

class GoombaFSM : public FiniteStateMachine
{
public:
    struct GoombaConfig : FiniteStateMachine::Config
    {
        F32 HorizontalSpeed{10.0f};
        F64 DeathTime{1.0};
        U32 Score{100};
        std::vector<std::string> DeathSensors{};
        std::vector<std::string> SwapSideSensors{};
    };
public:
    GoombaFSM(Scene& scene);
    void OnUpdate(F32 dt) override;
    void RegisterEntity(Entity e) override;
    void ReadConfig(const std::string& configPath) override;
    std::pair<Registry&, GoombaConfig&> GetRegistryConfigPair();
private:
    CollisionCallback::SensorCallback m_SensorCallback{nullptr};
};

class GoombaIdleState : public FSMState
{
public:
    GoombaIdleState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
};

class GoombaWalkState : public FSMState
{
public:
    GoombaWalkState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
private:
    bool m_SwitchSide{false};
};

class GoombaDeathState : public FSMState
{
public:
    GoombaDeathState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
private:
    bool m_WasDeleted{false};
};