#pragma once
#include "FSM.h"

// FSM for all block (level and coins)
class BlockFSM : public FiniteStateMachine
{
public:
    struct BlockConfig : FiniteStateMachine::Config
    {
        U32 SpawnedCoinScore{100};
        F64 SpawnedCoinLifeTime{1.0};
        F32 BlockVerticalSpeed{1.0f};
    };
public:
    BlockFSM(Scene& scene);
    void OnUpdate(F32 dt) override;
    void RegisterEntity(Entity e) override;
    void ReadConfig(const std::string& configPath) override;
    std::pair<Registry&, BlockConfig&> GetRegistryConfigPair();
private:
    CollisionCallback::SensorCallback m_SensorCallback{nullptr};
};

class DefaultBlockState : public FSMState
{
public:
    DefaultBlockState(bool hasCoin, Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    void OnLeave() override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
private:
    bool m_HasCoins{false};
};

class BumpedBlockState : public FSMState
{
public:
    BumpedBlockState(bool hadCoin, Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
private:
    F32 m_OriginalHeight{0.0f};
    bool m_ReachedApex{false};
    bool m_HadCoin{false};
};

class EmptyBlockState : public FSMState
{
public:
    EmptyBlockState(bool hadCoin, Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
private:
    bool m_HadCoin{false};
};

class DefaultCoinState : public FSMState
{
public:
    DefaultCoinState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
private:
    bool m_WasPickedUp{false};
};

class LaunchedCoinState : public FSMState
{
public:
    LaunchedCoinState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
};



