#pragma once

#include "FSM.h"

class KoopaFSM : public FiniteStateMachine
{
public:
    struct KoopaConfig : FiniteStateMachine::Config
    {
        F32 HorizontalSpeed{1.0f};
        F32 AKMHorizontalSpeed{10};
        F64 DeathTime{1.0};
        F32 DeathImpulse{1.0f};
        U32 Score{100};
        std::vector<std::string> DeathSensors{};
        std::vector<std::string> SwapSideSensors{};
    };
public:
    KoopaFSM(Scene& scene);
    void OnUpdate(F32 dt) override;
    void RegisterEntity(Entity e) override;
    void ReadConfig(const std::string& configPath) override;
    std::pair<Registry&, KoopaConfig&> GetRegistryConfigPair();
private:
    CollisionCallback::SensorCallback m_SensorCallback{nullptr};
};

class KoopaIdleState : public FSMState
{
public:
    KoopaIdleState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
};

class KoopaActiveState : public FSMState
{
public:
    KoopaActiveState(const std::string& name, Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
};

class KoopaWalkState : public KoopaActiveState
{
public:
    KoopaWalkState(Entity e, FiniteStateMachine& fsm);
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
private:
    bool m_SwitchSide{false};
};

class KoopaUpsideDownState : public KoopaActiveState
{
public:
    KoopaUpsideDownState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
};

class KoopaAbsoluteKillingMachineState : public KoopaActiveState
{
public:
    KoopaAbsoluteKillingMachineState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
private:
    bool m_SwitchSide{false};
};

class KoopaDeathState : public FSMState
{
public:
    KoopaDeathState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
};
