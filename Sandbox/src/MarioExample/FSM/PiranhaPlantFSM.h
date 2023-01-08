#pragma once
#include "FSM.h"

class PiranhaPlantFSM : public FiniteStateMachine
{
public:
    struct PiranhaPlantConfig : FiniteStateMachine::Config
    {
        F32 VerticalSpeed{1.0f};
        F32 Amplitude{1.0f};
    };
public:
    PiranhaPlantFSM(Scene& scene);
    void OnUpdate(F32 dt) override;
    void RegisterEntity(Entity e) override;
    void ReadConfig(const std::string& configPath) override;
    std::pair<Registry&, PiranhaPlantConfig&> GetRegistryConfigPair();
private:
    CollisionCallback::SensorCallback m_SensorCallback{nullptr};
};

class PiranhaPlantIdleState : public FSMState
{
public:
    PiranhaPlantIdleState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
};

class PiranhaPlantMoveState : public FSMState
{
public:
    PiranhaPlantMoveState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
};

class PiranhaPlantDeathState : public FSMState
{
public:
    PiranhaPlantDeathState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
};