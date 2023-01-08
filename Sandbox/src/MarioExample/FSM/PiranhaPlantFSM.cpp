#include "PiranhaPlantFSM.h"

#include "../MarioTags.h"

#include "../MarioScene.h"

PiranhaPlantFSM::PiranhaPlantFSM(Scene& scene)
    : FiniteStateMachine(scene)
{
    m_Config = CreateRef<PiranhaPlantConfig>();
    FiniteStateMachine::SetDefaultCollisionResponse(m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("PiranhaPlant", m_SensorCallback);
}

void PiranhaPlantFSM::OnUpdate(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<MarioPiranhaPlantTag, Component::FSMStateComp>(registry))
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

void PiranhaPlantFSM::RegisterEntity(Entity e)
{
    auto& registry = m_Scene.GetRegistry();
    auto& stateComp = registry.AddOrGet<Component::FSMStateComp>(e);
    stateComp.CurrentState = CreateRef<PiranhaPlantIdleState>(e, *this);
    stateComp.CurrentState->OnEnter(nullptr);
}

void PiranhaPlantFSM::ReadConfig(const std::string& configPath)
{
    FiniteStateMachine::ReadConfig(configPath);
    YAML::Node nodes = YAML::LoadFile(configPath);
    PiranhaPlantConfig& config = *static_cast<PiranhaPlantConfig*>(m_Config.get());
    config.VerticalSpeed = nodes["VerticalSpeed"].as<F32>();
    config.Amplitude = nodes["Amplitude"].as<F32>();
}

std::pair<Registry&, PiranhaPlantFSM::PiranhaPlantConfig&> PiranhaPlantFSM::GetRegistryConfigPair()
{
    return std::make_pair(
        std::reference_wrapper<Registry>(GetScene().GetRegistry()),
        std::reference_wrapper<PiranhaPlantConfig>(*static_cast<PiranhaPlantConfig*>(GetConfig())));
}

PiranhaPlantIdleState::PiranhaPlantIdleState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Idle", e, fsm)
{
}

void PiranhaPlantIdleState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

Ref<FSMState> PiranhaPlantIdleState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<PiranhaPlantFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioAwakeTag>(other)) return CreateRef<PiranhaPlantMoveState>(m_Entity, m_FSM);
    }
    return nullptr;
}

PiranhaPlantMoveState::PiranhaPlantMoveState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Move", e, fsm)
{
}

void PiranhaPlantMoveState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

Ref<FSMState> PiranhaPlantMoveState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<PiranhaPlantFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioKillTag>(other)) return CreateRef<PiranhaPlantDeathState>(m_Entity, m_FSM);
    }
    return nullptr;
}

Ref<FSMState> PiranhaPlantMoveState::OnUpdate(F32 dt)
{
    auto&& [registry, config] = m_FSM.As<PiranhaPlantFSM>().GetRegistryConfigPair();
    auto& tf = registry.Get<Component::LocalToParentTransform2D>(m_Entity);
    tf.Position.y = Math::Min(0.0f, config.Amplitude * std::sin(static_cast<F32>(Time::Get()) * dt * config.VerticalSpeed));
    return nullptr;
}

PiranhaPlantDeathState::PiranhaPlantDeathState(Entity e, FiniteStateMachine& fsm)
    : FSMState("Death", e, fsm)
{
}

void PiranhaPlantDeathState::OnEnter(FSMState* previousState)
{
    auto&& [registry, config] = m_FSM.As<PiranhaPlantFSM>().GetRegistryConfigPair();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
    auto& killSystem = registry.Add<Component::LifeTimeComponent>(m_Entity);
    killSystem.LifeTime = killSystem.LifeTimeLeft = 0.0;
}