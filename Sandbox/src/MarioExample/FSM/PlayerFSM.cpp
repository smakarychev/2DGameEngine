#include "PlayerFSM.h"

#include "../MarioScene.h"
#include "../Controllers/ControllersCommon.h"

PlayerFSM::PlayerFSM(Scene& scene)
    : FiniteStateMachine(scene)
{
    m_Config = CreateRef<PlayerConfig>();
    m_SensorCallback = [](
       Registry* registry, const CollisionCallback::CollisionData& collisionData,
       [[maybe_unused]] const Physics::ContactInfo2D& contact)
    {
        Entity primary = collisionData.Primary;
        Entity parent = registry->Get<Component::ParentRel>(primary).Parent;
        auto& stateComp = registry->Get<Component::FSMStateComp>(parent);
        Ref<FSMState> newState = stateComp.CurrentState->OnCollision(collisionData);
        if (newState != nullptr)
        {
            stateComp.CurrentState->OnStateSwap(newState.get());
            stateComp.CurrentState = newState;
        }
    };
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Player", "Bottom sensor", m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Player", "Left sensor", m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Player", "Right sensor", m_SensorCallback);
}

void PlayerFSM::OnUpdate(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<MarioPlayerTag, Component::FSMStateComp>(registry))
    {
        auto& stateComp = registry.Get<Component::FSMStateComp>(e);
        Ref<FSMState> newState = stateComp.CurrentState->OnUpdate(dt);
        if (newState != nullptr)
        {
            stateComp.CurrentState->OnStateSwap(newState.get());
            stateComp.CurrentState = newState;
        }
    }
}

void PlayerFSM::RegisterEntity(Entity e)
{
    auto& registry = m_Scene.GetRegistry();
    auto& stateComp = registry.AddOrGet<Component::FSMStateComp>(e);
    stateComp.CurrentState = CreateRef<PlayerIdleState>(e, *this);
    stateComp.CurrentState->OnEnter(nullptr);
}

void PlayerFSM::ReadConfig(const std::string& configPath)
{
    FiniteStateMachine::ReadConfig(configPath);
    YAML::Node nodes = YAML::LoadFile(configPath);
    PlayerConfig& config = *static_cast<PlayerConfig*>(m_Config.get());
    config.JumpImpulse = nodes["JumpImpulse"].as<F32>();
    config.MaxHorizontalSpeed = nodes["MaxHorizontalSpeed"].as<F32>();
    config.MaxFallSpeed = nodes["MaxFallSpeed"].as<F32>();
    config.VerticalVelocityDrop = nodes["VerticalVelocityDrop"].as<F32>();
    config.HorizontalVelocityDrop = nodes["HorizontalVelocityDrop"].as<F32>();
    config.CoyoteTime = nodes["CoyoteTime"].as<F64>();
    config.JumpBuffer = nodes["JumpBuffer"].as<F64>();
    config.DeathTime = nodes["DeathTime"].as<F64>();
    auto deathSensors = nodes["DeathSensors"];
    for (auto ds : deathSensors) config.DeathSensors.push_back(ds["Name"].as<std::string>());
    auto groundSensors = nodes ["GroundSensors"];
    for (auto gs : groundSensors) config.GroundSensors.push_back(gs["Name"].as<std::string>());
}

I32 PlayerFSM::GetHorizontalMoveDir()
{
    I32 move = 0;
    if (Input::GetKey(Key::A)) move = -1;
    if (Input::GetKey(Key::D)) move = 1;
    return move;
}

void PlayerFSM::FlipSpriteBasedOnMoveDir(Entity e, I32 moveDir)
{
    auto& registry = m_Scene.GetRegistry();
    auto& spr = registry.Get<Component::SpriteRenderer>(e);
    if (moveDir == 1) spr.FlipX = false;
    else if (moveDir == -1) spr.FlipX = true;
}

std::pair<Registry&, PlayerFSM::PlayerConfig&> PlayerFSM::GetRegistryConfigPair()
{
    return std::make_pair(
        std::reference_wrapper<Registry>(GetScene().GetRegistry()),
        std::reference_wrapper<PlayerConfig>(*static_cast<PlayerFSM::PlayerConfig*>(GetConfig())));
}

PlayerMoveState::PlayerMoveState(const std::string& name, Entity e, FiniteStateMachine& fsm)
    : FSMState(name, e, fsm)
{
}

Ref<FSMState> PlayerMoveState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<PlayerFSM>().GetRegistryConfigPair();
    
    Entity other = collision.Secondary;
    if (!registry.Has<MarioEnemyTag>(other)) return nullptr;
    
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        Entity sensor = collision.Primary;
        auto& sensorName = registry.Get<Component::Name>(sensor).EntityName;
        auto it = std::ranges::find(config.DeathSensors, sensorName);
        if (it == config.DeathSensors.end()) return nullptr;

        return CreateRef<PlayerDeathState>(m_Entity, m_FSM);
    }
    return nullptr;
}

PlayerDeathState::PlayerDeathState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Death", e, fsm)
{
}

void PlayerDeathState::OnEnter(FSMState* previousState)
{
    auto&& [registry, config] = m_FSM.As<PlayerFSM>().GetRegistryConfigPair();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = m_FSM.GetAnimationsMap()[m_Name];
    auto& killSystem = registry.Add<Component::KillComponent>(m_Entity);
    killSystem.LifeTime = config.DeathTime;
    ControllerUtils::OnDeathImpulse(m_Entity, {0.0f, config.JumpImpulse}, m_FSM.GetScene());
}

PlayerGroundMoveState::PlayerGroundMoveState(const std::string& name, Entity e, FiniteStateMachine& fsm)
    : PlayerMoveState(name, e, fsm)
{
}

void PlayerGroundMoveState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = m_FSM.GetAnimationsMap()[m_Name];
}

Ref<FSMState> PlayerGroundMoveState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    Ref<FSMState> parentCollision = PlayerMoveState::OnCollision(collision);
    if (parentCollision != nullptr) return parentCollision;
    
    auto&& [registry, config] = m_FSM.As<PlayerFSM>().GetRegistryConfigPair();

    Entity other = collision.Secondary;
    if (!registry.Has<MarioLevelTag>(other)) return nullptr;
    
    Entity sensor = collision.Primary;
    auto& sensorName = registry.Get<Component::Name>(sensor).EntityName;
    auto it = std::ranges::find(config.GroundSensors, sensorName);
    if (it == config.GroundSensors.end()) return nullptr;

    if (collision.ContactState == Physics::ContactListener::ContactState::Begin) m_GroundCollisions++;
    else m_GroundCollisions--;

    return nullptr;
}

Ref<FSMState> PlayerGroundMoveState::OnUpdate(F32 dt)
{
    if (Input::GetKeyDown(Key::Space)) return CreateRef<PlayerJumpState>(m_Entity, m_FSM);
    if (m_GroundCollisions == 0)
        return CreateRef<PlayerFallState>(m_Entity, m_FSM);
    return nullptr;
}

PlayerAirMoveState::PlayerAirMoveState(const std::string& name, Entity e, FiniteStateMachine& fsm)
    : PlayerMoveState(name, e, fsm)
{
}

void PlayerAirMoveState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = m_FSM.GetAnimationsMap()[m_Name];
}

Ref<FSMState> PlayerAirMoveState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    Ref<FSMState> parentCollision = PlayerMoveState::OnCollision(collision);
    if (parentCollision != nullptr) return parentCollision;
    
    auto&& [registry, config] = m_FSM.As<PlayerFSM>().GetRegistryConfigPair();

    Entity other = collision.Secondary;
    if (!registry.Has<MarioLevelTag>(other)) return nullptr;
    
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        Entity sensor = collision.Primary;
        auto& sensorName = registry.Get<Component::Name>(sensor).EntityName;
        auto it = std::ranges::find(config.GroundSensors, sensorName);
        if (it == config.GroundSensors.end()) return nullptr;

        m_HitGround = true;
    }
    return nullptr;
}

PlayerWalkState::PlayerWalkState(Entity e, FiniteStateMachine& fsm)
    : PlayerGroundMoveState("Walk", e, fsm)
{
}

void PlayerWalkState::OnEnter(FSMState* previousState)
{
    PlayerGroundMoveState::OnEnter(previousState);
    if (previousState && previousState->GetName() == "Idle") m_GroundCollisions =
        ((PlayerGroundMoveState*)(previousState))->GetCollisionCount();
}

void PlayerIdleState::OnEnter(FSMState* previousState)
{
    PlayerGroundMoveState::OnEnter(previousState);
    if (previousState && previousState->GetName() == "Walk") m_GroundCollisions =
        ((PlayerGroundMoveState*)(previousState))->GetCollisionCount();
}

Ref<FSMState> PlayerWalkState::OnUpdate(F32 dt)
{
    Ref<FSMState> parentUpdate = PlayerGroundMoveState::OnUpdate(dt);
    if (parentUpdate != nullptr) return parentUpdate;

    auto&& [registry, config] = m_FSM.As<PlayerFSM>().GetRegistryConfigPair();
    
    auto& rb = registry.Get<Component::RigidBody2D>(m_Entity);
    auto& vel = rb.PhysicsBody->GetLinearVelocity();

    I32 move = m_FSM.As<PlayerFSM>().GetHorizontalMoveDir();
    m_FSM.As<PlayerFSM>().FlipSpriteBasedOnMoveDir(m_Entity, move);
    rb.PhysicsBody->AddForce(glm::vec2{90.0f * static_cast<F32>(move), 0.0f});
    if (move == 0) vel.x *= config.HorizontalVelocityDrop;
    vel.x = Math::Clamp(vel.x, -config.MaxHorizontalSpeed, config.MaxHorizontalSpeed);

    if (Math::Abs(vel.x) < 0.1f && move == 0) return CreateRef<PlayerIdleState>(m_Entity, m_FSM);
    
    return nullptr;
}

PlayerIdleState::PlayerIdleState(Entity e, FiniteStateMachine& fsm)
    : PlayerGroundMoveState("Idle", e, fsm)
{
}

Ref<FSMState> PlayerIdleState::OnUpdate(F32 dt)
{
    Ref<FSMState> parentUpdate = PlayerGroundMoveState::OnUpdate(dt);
    if (parentUpdate != nullptr) return parentUpdate;

    if (Input::GetKey(Key::A) || Input::GetKey(Key::D)) return CreateRef<PlayerWalkState>(m_Entity, m_FSM);
    
    return nullptr;
}

PlayerFallState::PlayerFallState(Entity e, FiniteStateMachine& fsm)
    : PlayerAirMoveState("Fall", e, fsm)
{
}

void PlayerFallState::OnEnter(FSMState* previousState)
{
    PlayerAirMoveState::OnEnter(previousState);
    auto&& [registry, config] = m_FSM.As<PlayerFSM>().GetRegistryConfigPair();
    if (!previousState || previousState->GetName() != "Jump") m_CoyoteTimeCounter = config.CoyoteTime;
}

Ref<FSMState> PlayerFallState::OnUpdate(F32 dt)
{
    Ref<FSMState> parentUpdate = PlayerAirMoveState::OnUpdate(dt);
    if (parentUpdate != nullptr) return parentUpdate;
    
    auto&& [registry, config] = m_FSM.As<PlayerFSM>().GetRegistryConfigPair();
    
    m_CoyoteTimeCounter -= static_cast<F64>(dt);
    
    if (Input::GetKeyDown(Key::Space)) m_JumpBufferTimeCounter = config.JumpBuffer;
    else m_JumpBufferTimeCounter -= static_cast<F64>(dt);
    
    auto& rb = registry.Get<Component::RigidBody2D>(m_Entity);
    auto& vel = rb.PhysicsBody->GetLinearVelocity();

    // The idea is that we could apply different horizontal forces in fall/jump/walk state.
    I32 move = m_FSM.As<PlayerFSM>().GetHorizontalMoveDir();
    m_FSM.As<PlayerFSM>().FlipSpriteBasedOnMoveDir(m_Entity, move);
    rb.PhysicsBody->AddForce(glm::vec2{90.0f * static_cast<F32>(move), 0.0f});
    if (move == 0) vel.x *= config.HorizontalVelocityDrop;
    vel.x = Math::Clamp(vel.x, -config.MaxHorizontalSpeed, config.MaxHorizontalSpeed);

    if (m_CoyoteTimeCounter > 0.0 && m_JumpBufferTimeCounter > 0.0)
    {
        return CreateRef<PlayerJumpState>(m_Entity, m_FSM);
    }
    if (m_HitGround)
    {
        if (m_JumpBufferTimeCounter > 0.0) return CreateRef<PlayerJumpState>(m_Entity, m_FSM);
        if (move != 0) return CreateRef<PlayerWalkState>(m_Entity, m_FSM);
        return CreateRef<PlayerIdleState>(m_Entity, m_FSM);
    }
    vel.y = Math::Clamp(vel.y, config.MaxFallSpeed, std::numeric_limits<F32>::max());
    return nullptr;
}

PlayerJumpState::PlayerJumpState(Entity e, FiniteStateMachine& fsm)
    : PlayerAirMoveState("Jump", e, fsm)
{
}

void PlayerJumpState::OnEnter(FSMState* previousState)
{
    PlayerAirMoveState::OnEnter(previousState);
    PlayerFSM::PlayerConfig& config = *static_cast<PlayerFSM::PlayerConfig*>(m_FSM.GetConfig());
    auto& rb = m_FSM.GetScene().GetRegistry().Get<Component::RigidBody2D>(m_Entity);
    auto& vel = rb.PhysicsBody->GetLinearVelocity();
    vel.y = 0.0f;
    rb.PhysicsBody->AddForce(glm::vec2{0.0f, config.JumpImpulse}, Physics::ForceMode::Impulse);
}

Ref<FSMState> PlayerJumpState::OnUpdate(F32 dt)
{
    Ref<FSMState> parentUpdate = PlayerAirMoveState::OnUpdate(dt);
    if (parentUpdate != nullptr) return parentUpdate;
    
    auto&& [registry, config] = m_FSM.As<PlayerFSM>().GetRegistryConfigPair();

    auto& rb = registry.Get<Component::RigidBody2D>(m_Entity);
    auto& vel = rb.PhysicsBody->GetLinearVelocity();
        
    if (Input::GetKeyUp(Key::Space)) vel.y *= config.VerticalVelocityDrop;
    if (vel.y < 0.0f) return CreateRef<PlayerFallState>(m_Entity, m_FSM);

    I32 move = m_FSM.As<PlayerFSM>().GetHorizontalMoveDir();
    m_FSM.As<PlayerFSM>().FlipSpriteBasedOnMoveDir(m_Entity, move);
    rb.PhysicsBody->AddForce(glm::vec2{90.0f * static_cast<F32>(move), 0.0f});
    if (move == 0) vel.x *= config.HorizontalVelocityDrop;
    vel.x = Math::Clamp(vel.x, -config.MaxHorizontalSpeed, config.MaxHorizontalSpeed);
    
    return nullptr;
}
