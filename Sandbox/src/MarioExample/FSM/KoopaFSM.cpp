#include "KoopaFSM.h"

#include "../MarioTags.h"

#include "../MarioScene.h"

#include "FSMCommon.h"
#include "../MarioSceneUtils.h"

KoopaFSM::KoopaFSM(Scene& scene)
    : FiniteStateMachine(scene)
{
    m_Config = CreateRef<KoopaConfig>();
    FiniteStateMachine::SetDefaultCollisionResponse(m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Koopa", m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Koopa", m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Koopa", m_SensorCallback);
}

void KoopaFSM::OnUpdate(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<MarioKoopaTag, Component::FSMStateComp>(registry))
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

void KoopaFSM::RegisterEntity(Entity e)
{
    auto& registry = m_Scene.GetRegistry();
    auto& stateComp = registry.AddOrGet<Component::FSMStateComp>(e);
    stateComp.CurrentState = CreateRef<KoopaIdleState>(e, *this);
    stateComp.CurrentState->OnEnter(nullptr);
}

void KoopaFSM::ReadConfig(const std::string& configPath)
{
    FiniteStateMachine::ReadConfig(configPath);
    YAML::Node nodes = YAML::LoadFile(configPath);
    KoopaConfig& config = *static_cast<KoopaConfig*>(m_Config.get());
    config.HorizontalSpeed = nodes["HorizontalSpeed"].as<F32>();
    config.AKMHorizontalSpeed = nodes["AKMHorizontalSpeed"].as<F32>();
    config.DeathImpulse = nodes["DeathImpulse"].as<F32>();
    config.DeathTime = nodes["DeathTime"].as<F64>();
    config.Score = nodes["Score"].as<U32>();
    auto deathSensors = nodes["DeathSensors"];
    for (auto ds : deathSensors) config.DeathSensors.push_back(ds["Name"].as<std::string>());
    auto swapSideSensors = nodes ["SwapSideSensors"];
    for (auto sss : swapSideSensors) config.SwapSideSensors.push_back(sss["Name"].as<std::string>());
}

std::pair<Registry&, KoopaFSM::KoopaConfig&> KoopaFSM::GetRegistryConfigPair()
{
    return std::make_pair(
    std::reference_wrapper<Registry>(GetScene().GetRegistry()),
    std::reference_wrapper<KoopaConfig>(*static_cast<KoopaConfig*>(GetConfig())));
}

KoopaIdleState::KoopaIdleState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Idle", e, fsm)
{
    
}

void KoopaIdleState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

Ref<FSMState> KoopaIdleState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioAwakeTag>(other)) return CreateRef<KoopaWalkState>(m_Entity, m_FSM);
    }
    return nullptr;
}

KoopaActiveState::KoopaActiveState(const std::string& name, Entity e, FiniteStateMachine& fsm)
    : FSMState(name, e, fsm)
{
}

void KoopaActiveState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

Ref<FSMState> KoopaActiveState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioKillTag>(other))
        {
            if (!registry.Has<MarioLevelTag>(other)) MarioSceneUtils::SpawnScoreEntity(m_FSM.GetScene(), config.Score, m_Entity);
            return CreateRef<KoopaDeathState>(m_Entity, m_FSM);
        }
    }
    return nullptr;
}

KoopaWalkState::KoopaWalkState(Entity e, FiniteStateMachine& fsm)
    : KoopaActiveState("Walk", e, fsm)
{
}

Ref<FSMState> KoopaWalkState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto parentCollision = KoopaActiveState::OnCollision(collision);
    if (parentCollision != nullptr) return parentCollision;
    
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioPlayerTag>(other) && !registry.Has<MarioHarmlessTag>(other))
        {
            Entity sensor = collision.Primary;
            auto& sensorName = registry.Get<Component::Name>(sensor).EntityName;
            auto it = std::ranges::find(config.DeathSensors, sensorName);
            if (it == config.DeathSensors.end()) return nullptr;

            MarioSceneUtils::SpawnScoreEntity(m_FSM.GetScene(), config.Score, m_Entity);
            
            return CreateRef<KoopaUpsideDownState>(m_Entity, m_FSM);
        }
        if (registry.Has<MarioLevelTag>(other) || registry.Has<MarioEnemyTag>(other)) m_SwitchSide = !m_SwitchSide;
    }
    return nullptr;
}

Ref<FSMState> KoopaWalkState::OnUpdate(F32 dt)
{
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    auto& rb = registry.Get<Component::RigidBody2D>(m_Entity);
    auto& vel = rb.PhysicsBody->GetLinearVelocity();
    I32 move = m_SwitchSide ? 1 : -1;
    FSMUtils::FlipSpriteBasedOnMoveDir(m_FSM.GetScene(), m_Entity, move);
    vel.x = static_cast<F32>(move) * config.HorizontalSpeed;
    return nullptr;
}

KoopaUpsideDownState::KoopaUpsideDownState(Entity e, FiniteStateMachine& fsm)
    : KoopaActiveState("UpsideDown", e, fsm)
{
}

void KoopaUpsideDownState::OnEnter(FSMState* previousState)
{
    KoopaActiveState::OnEnter(previousState);
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    registry.AddOrGet<MarioHarmlessTag>(m_Entity);
}

Ref<FSMState> KoopaUpsideDownState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto parentCollision = KoopaActiveState::OnCollision(collision);
    if (parentCollision != nullptr) return parentCollision;
    
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioPlayerTag>(other) && !registry.Has<MarioHarmlessTag>(other))
        {
            Entity sensor = collision.Primary;
            auto& sensorName = registry.Get<Component::Name>(sensor).EntityName;
            auto it = std::ranges::find(config.DeathSensors, sensorName);
            if (it == config.DeathSensors.end()) return nullptr;

            MarioSceneUtils::SpawnScoreEntity(m_FSM.GetScene(), config.Score, m_Entity);
            
            return CreateRef<KoopaAbsoluteKillingMachineState>(m_Entity, m_FSM);
        }
    }
    return nullptr;
}

KoopaAbsoluteKillingMachineState::KoopaAbsoluteKillingMachineState(Entity e, FiniteStateMachine& fsm)
    : KoopaActiveState("AKM", e, fsm)
{
}

void KoopaAbsoluteKillingMachineState::OnEnter(FSMState* previousState)
{
    KoopaActiveState::OnEnter(previousState);
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    registry.Add<MarioKillTag>(m_Entity);
}

Ref<FSMState> KoopaAbsoluteKillingMachineState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto parentCollision = KoopaActiveState::OnCollision(collision);
    if (parentCollision != nullptr) return parentCollision;

    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioLevelTag>(other) || registry.Has<MarioEnemyTag>(other)) m_SwitchSide = !m_SwitchSide;
    }
    return nullptr;
}

Ref<FSMState> KoopaAbsoluteKillingMachineState::OnUpdate(F32 dt)
{
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    auto& rb = registry.Get<Component::RigidBody2D>(m_Entity);
    auto& vel = rb.PhysicsBody->GetLinearVelocity();
    I32 move = m_SwitchSide ? 1 : -1;
    FSMUtils::FlipSpriteBasedOnMoveDir(m_FSM.GetScene(), m_Entity, move);
    vel.x = static_cast<F32>(move) * config.AKMHorizontalSpeed;
    return nullptr;
}

KoopaDeathState::KoopaDeathState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Death", e, fsm)
{
}

void KoopaDeathState::OnEnter(FSMState* previousState)
{
    auto&& [registry, config] = m_FSM.As<KoopaFSM>().GetRegistryConfigPair();
    registry.AddOrGet<MarioHarmlessTag>(m_Entity);
    if (registry.Has<MarioKillTag>(m_Entity)) registry.Remove<MarioKillTag>(m_Entity);
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
    auto& killSystem = registry.Add<Component::LifeTimeComponent>(m_Entity);
    killSystem.LifeTime = killSystem.LifeTimeLeft = config.DeathTime;
    FSMUtils::OnDeathImpulse(m_Entity, {0.0f, config.DeathImpulse}, m_FSM.GetScene());
}