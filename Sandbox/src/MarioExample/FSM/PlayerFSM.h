#pragma once
#include "FSM.h"
#include "../MarioTags.h"
class PlayerFSM : public FiniteStateMachine
{
public:
    struct PlayerConfig : FiniteStateMachine::Config
    {
        F32 MaxHorizontalSpeed{10.0f};
        F32 MaxFallSpeed{-10.0f};
        F32 JumpImpulse{20.0f};
        F32 VerticalVelocityDrop{0.5f};
        F32 HorizontalVelocityDrop{0.5f};
        F64 CoyoteTime{0.5};
        F64 JumpBuffer{0.5};
        F64 DeathTime{1.0};
        std::vector<std::string> DeathSensors{};
        std::vector<std::string> GroundSensors{};
    };
public:
    PlayerFSM(Scene& scene);
    void OnUpdate(F32 dt) override;
    void RegisterEntity(Entity e) override;
    void ReadConfig(const std::string& configPath) override;
    I32 GetHorizontalMoveDir();
    void FlipSpriteBasedOnMoveDir(Entity e, I32 moveDir);
    std::pair<Registry&, PlayerConfig&> GetRegistryConfigPair();
private:
    CollisionCallback::SensorCallback m_SensorCallback{nullptr};
};

// Base class for all player movements states (walk, idle, jump, fall).
class PlayerMoveState : public FSMState
{
public:
    PlayerMoveState(const std::string& name, Entity e, FiniteStateMachine& fsm);
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
};

class PlayerDeathState : public FSMState
{
public:
    PlayerDeathState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
};

class PlayerGroundMoveState : public PlayerMoveState
{
public:
    PlayerGroundMoveState(const std::string& name, Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
    U32 GetCollisionCount() const { return m_GroundCollisions; }
protected:
    U32 m_GroundCollisions{1};
};

class PlayerAirMoveState : public PlayerMoveState
{
public:
    PlayerAirMoveState(const std::string& name, Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision) override;
protected:
    bool m_HitGround{false};
};

class PlayerWalkState : public PlayerGroundMoveState
{
public:
    PlayerWalkState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
};

class PlayerIdleState : public PlayerGroundMoveState
{
public:
    PlayerIdleState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
};

class PlayerFallState : public PlayerAirMoveState
{
public:
    PlayerFallState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
private:
    F64 m_CoyoteTimeCounter{0.0};
    F64 m_JumpBufferTimeCounter{0.0};
};

class PlayerJumpState : public PlayerAirMoveState
{
public:
    PlayerJumpState(Entity e, FiniteStateMachine& fsm);
    void OnEnter(FSMState* previousState) override;
    Ref<FSMState> OnUpdate(F32 dt) override;
};