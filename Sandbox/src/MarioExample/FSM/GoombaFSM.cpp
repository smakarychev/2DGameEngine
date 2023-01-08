#include "GoombaFSM.h"

#include "../MarioTags.h"

#include "../MarioScene.h"
#include "../MarioSceneUtils.h"

#include "FSMCommon.h"

GoombaFSM::GoombaFSM(Scene& scene)
    : FiniteStateMachine(scene)
{
    m_Config = CreateRef<GoombaConfig>();
    FiniteStateMachine::SetDefaultCollisionResponse(m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Goomba", m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Goomba", m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Goomba", m_SensorCallback);
}

void GoombaFSM::OnUpdate(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<MarioGoombaTag, Component::FSMStateComp>(registry))
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

void GoombaFSM::RegisterEntity(Entity e)
{
    auto& registry = m_Scene.GetRegistry();
    auto& stateComp = registry.AddOrGet<Component::FSMStateComp>(e);
    stateComp.CurrentState = CreateRef<GoombaIdleState>(e, *this);
    stateComp.CurrentState->OnEnter(nullptr);
}

void GoombaFSM::ReadConfig(const std::string& configPath)
{
    FiniteStateMachine::ReadConfig(configPath);
    YAML::Node nodes = YAML::LoadFile(configPath);
    GoombaConfig& config = *static_cast<GoombaConfig*>(m_Config.get());
    config.HorizontalSpeed = nodes["HorizontalSpeed"].as<F32>();
    config.DeathTime = nodes["DeathTime"].as<F64>();
    config.Score = nodes["Score"].as<U32>();
    auto deathSensors = nodes["DeathSensors"];
    for (auto ds : deathSensors) config.DeathSensors.push_back(ds["Name"].as<std::string>());
    auto swapSideSensors = nodes ["SwapSideSensors"];
    for (auto sss : swapSideSensors) config.SwapSideSensors.push_back(sss["Name"].as<std::string>());
}

std::pair<Registry&, GoombaFSM::GoombaConfig&> GoombaFSM::GetRegistryConfigPair()
{
    return std::make_pair(
        std::reference_wrapper<Registry>(GetScene().GetRegistry()),
        std::reference_wrapper<GoombaConfig>(*static_cast<GoombaConfig*>(GetConfig())));
}

GoombaIdleState::GoombaIdleState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Idle", e, fsm)
{
}

void GoombaIdleState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

Ref<FSMState> GoombaIdleState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<GoombaFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioAwakeTag>(other)) return CreateRef<GoombaWalkState>(m_Entity, m_FSM);
    }
    return nullptr;
}

GoombaWalkState::GoombaWalkState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Walk", e, fsm)
{
}

void GoombaWalkState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

Ref<FSMState> GoombaWalkState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<GoombaFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioKillTag>(other))
        {
            if (!registry.Has<MarioLevelTag>(other)) MarioSceneUtils::SpawnScoreEntity(m_FSM.GetScene(), config.Score, m_Entity);
            return CreateRef<GoombaDeathState>(m_Entity, m_FSM);
        }
        if (registry.Has<MarioPlayerTag>(other) && !registry.Has<MarioHarmlessTag>(other))
        {
            Entity sensor = collision.Primary;
            auto& sensorName = registry.Get<Component::Name>(sensor).EntityName;
            auto it = std::ranges::find(config.DeathSensors, sensorName);
            if (it == config.DeathSensors.end()) return nullptr;

            MarioSceneUtils::SpawnScoreEntity(m_FSM.GetScene(), config.Score, m_Entity);
            
            return CreateRef<GoombaDeathState>(m_Entity, m_FSM);
        }
        if (registry.Has<MarioLevelTag>(other) || registry.Has<MarioEnemyTag>(other)) m_SwitchSide = !m_SwitchSide;
    }
    return nullptr;
}

Ref<FSMState> GoombaWalkState::OnUpdate(F32 dt)
{
    auto&& [registry, config] = m_FSM.As<GoombaFSM>().GetRegistryConfigPair();

    auto& rb = registry.Get<Component::RigidBody2D>(m_Entity);
    auto& vel = rb.PhysicsBody->GetLinearVelocity();
    I32 move = m_SwitchSide ? 1 : -1;
    FSMUtils::FlipSpriteBasedOnMoveDir(m_FSM.GetScene(), m_Entity, move);
    vel.x = static_cast<F32>(move) * config.HorizontalSpeed;
    
    return nullptr;
}

GoombaDeathState::GoombaDeathState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Death", e, fsm)
{
}

void GoombaDeathState::OnEnter(FSMState* previousState)
{
    auto&& [registry, config] = m_FSM.As<GoombaFSM>().GetRegistryConfigPair();
    registry.Add<MarioHarmlessTag>(m_Entity);
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
    auto& killSystem = registry.Add<Component::LifeTimeComponent>(m_Entity);
    killSystem.LifeTime = killSystem.LifeTimeLeft = config.DeathTime;
}

Ref<FSMState> GoombaDeathState::OnUpdate(F32 dt)
{
    // Basically physics engine really doesn't like when we delete colliders / rb during physics step.
    if (m_WasDeleted) return nullptr;
    auto& registry = m_FSM.GetScene().GetRegistry();
    registry.Remove<Component::BoxCollider2D>(m_Entity);
    registry.Remove<Component::RigidBody2D>(m_Entity);
    m_WasDeleted = true;
    return nullptr;
}