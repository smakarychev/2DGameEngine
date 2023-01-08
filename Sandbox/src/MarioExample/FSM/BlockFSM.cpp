#include "BlockFSM.h"

#include "../MarioScene.h"
#include "../MarioSceneUtils.h"

BlockFSM::BlockFSM(Scene& scene)
    : FiniteStateMachine(scene)
{
    m_Config = CreateRef<BlockConfig>();
    FiniteStateMachine::SetDefaultCollisionResponse(m_SensorCallback);
    static_cast<MarioScene&>(m_Scene).AddSensorCallback("Block", m_SensorCallback);
}

void BlockFSM::OnUpdate(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<MarioLevelTag, Component::FSMStateComp>(registry))
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

void BlockFSM::RegisterEntity(Entity e)
{
    auto& registry = m_Scene.GetRegistry();
    auto& stateComp = registry.AddOrGet<Component::FSMStateComp>(e);
    if (registry.Has<MarioEmptyBlockTag>(e)) stateComp.CurrentState = CreateRef<DefaultBlockState>(false, e, *this);
    else if (registry.Has<MarioCoinBlockTag>(e)) stateComp.CurrentState = CreateRef<DefaultBlockState>(true, e, *this);
    else if (registry.Has<Component::LifeTimeComponent>(e)) stateComp.CurrentState = CreateRef<LaunchedCoinState>(e, *this);
    else stateComp.CurrentState = CreateRef<DefaultCoinState>(e, *this);
    stateComp.CurrentState->OnEnter(nullptr);
}

void BlockFSM::ReadConfig(const std::string& configPath)
{
    FiniteStateMachine::ReadConfig(configPath);
    YAML::Node nodes = YAML::LoadFile(configPath);
    BlockConfig& config = *static_cast<BlockConfig*>(m_Config.get());
    config.SpawnedCoinScore = nodes["SpawnedCoinScore"].as<U32>();
    config.SpawnedCoinLifeTime = nodes["SpawnedCoinLifeTime"].as<F64>();
    config.BlockVerticalSpeed = nodes["BlockVerticalSpeed"].as<F32>();
}

std::pair<Registry&, BlockFSM::BlockConfig&> BlockFSM::GetRegistryConfigPair()
{
    return std::make_pair(
        std::reference_wrapper<Registry>(GetScene().GetRegistry()),
        std::reference_wrapper<BlockConfig>(*static_cast<BlockConfig*>(GetConfig())));
}

DefaultBlockState::DefaultBlockState(bool hasCoin, Entity e, FiniteStateMachine& fsm)
    : FSMState("Block" + std::string(hasCoin ? "Coins" : "Empty"), e, fsm), m_HasCoins(hasCoin)
{
}

void DefaultBlockState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

void DefaultBlockState::OnLeave()
{
    if (m_HasCoins && Random::Float() > 0.45f)
    {
        auto&& [registry, config] = m_FSM.As<BlockFSM>().GetRegistryConfigPair();
        Entity coin = MarioSceneUtils::SpawnCoin(m_FSM.GetScene(), config.SpawnedCoinScore, m_Entity);
        auto& lifeTime = registry.Add<Component::LifeTimeComponent>(coin);
        lifeTime.LifeTime = lifeTime.LifeTimeLeft = config.SpawnedCoinLifeTime;
        m_FSM.RegisterEntity(coin);
    }
}

Ref<FSMState> DefaultBlockState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<BlockFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<Component::ParentRel>(other) &&
            registry.Has<MarioPlayerTag>(registry.Get<Component::ParentRel>(other).Parent))
        {
            const std::string& sensorName = registry.Get<Component::Name>(other).EntityName;
            if (sensorName == "Top sensor") return CreateRef<BumpedBlockState>(m_HasCoins, m_Entity, m_FSM);
        }
    }
    return nullptr;
}

BumpedBlockState::BumpedBlockState(bool hadCoin, Entity e, FiniteStateMachine& fsm)
    : FSMState("Bumped" + std::string(hadCoin ? "Coins" : "Empty"), e, fsm), m_HadCoin(hadCoin)
{
}

void BumpedBlockState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
    auto& tf = registry.Get<Component::LocalToParentTransform2D>(m_Entity);
    m_OriginalHeight = tf.Position.y;
}

Ref<FSMState> BumpedBlockState::OnUpdate(F32 dt)
{
    auto&& [registry, config] = m_FSM.As<BlockFSM>().GetRegistryConfigPair();

    I32 move = 1;
    if (m_ReachedApex) move = -1;

    auto& tf = registry.Get<Component::LocalToParentTransform2D>(m_Entity);
    tf.Position.y += move * config.BlockVerticalSpeed * dt;
    if (!m_ReachedApex && tf.Position.y > m_OriginalHeight + tf.Scale.y)
    {
        tf.Position.y = m_OriginalHeight + tf.Scale.y;
        m_ReachedApex = true;
    }
    else if (m_ReachedApex && tf.Position.y < m_OriginalHeight)
    {
        tf.Position.y = m_OriginalHeight;
        return CreateRef<EmptyBlockState>(m_HadCoin, m_Entity, m_FSM);
    }
    
    return nullptr;
}

EmptyBlockState::EmptyBlockState(bool hadCoin, Entity e, FiniteStateMachine& fsm)
    : FSMState("EmptyBlock" + std::string(hadCoin ? "Coins" : "Empty"), e, fsm), m_HadCoin(hadCoin)
{
}

void EmptyBlockState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

Ref<FSMState> EmptyBlockState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<BlockFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<Component::ParentRel>(other) &&
            registry.Has<MarioPlayerTag>(registry.Get<Component::ParentRel>(other).Parent))
        {
            const std::string& sensorName = registry.Get<Component::Name>(other).EntityName;
            if (sensorName == "Top sensor") return CreateRef<BumpedBlockState>(m_HadCoin, m_Entity, m_FSM);
        }
    }
    return nullptr;
}

DefaultCoinState::DefaultCoinState(Entity e, FiniteStateMachine& fsm)
    : FSMState("CoinDefault", e, fsm)
{
}

void DefaultCoinState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
}

Ref<FSMState> DefaultCoinState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    auto&& [registry, config] = m_FSM.As<BlockFSM>().GetRegistryConfigPair();
    Entity other = collision.Secondary;
    if (collision.ContactState == Physics::ContactListener::ContactState::Begin)
    {
        if (registry.Has<MarioPlayerTag>(other)) m_WasPickedUp = true;
    }
    return nullptr;
}

Ref<FSMState> DefaultCoinState::OnUpdate(F32 dt)
{
    auto&& [registry, config] = m_FSM.As<BlockFSM>().GetRegistryConfigPair();
    if (m_WasPickedUp)
    {
        MarioSceneUtils::SpawnScoreEntity(m_FSM.GetScene(), config.SpawnedCoinScore, m_Entity);
        SceneUtils::DeleteEntity(m_FSM.GetScene(), m_Entity);
    }
    return nullptr;
}

LaunchedCoinState::LaunchedCoinState(Entity e, FiniteStateMachine& fsm)
    : FSMState("CoinLaunched", e, fsm)
{
}

void LaunchedCoinState::OnEnter(FSMState* previousState)
{
    auto& registry = m_FSM.GetScene().GetRegistry();
    auto& animationComp = registry.Get<Component::Animation>(m_Entity);
    animationComp.SpriteAnimation = CreateRef<SpriteAnimation>(*m_FSM.GetAnimationsMap()[m_Name]);
    auto& rb = registry.Get<Component::RigidBody2D>(m_Entity);
    auto& vel = rb.PhysicsBody->GetLinearVelocity();
    vel.x = Random::Float(-2.0f, 2.0f);
    vel.y = Random::Float(0.0f, 10.0f);
}
