#include "FSM.h"

#include "FSMCommon.h"

FSMState::FSMState(const std::string& name, Entity e, FiniteStateMachine& fsm)
    : m_Name(name), m_Entity(e),
      m_FSM(fsm)
{
}

FSMState::~FSMState() = default;

void FSMState::OnEnter(FSMState* previousState)
{
}

void FSMState::OnLeave()
{
}

Ref<FSMState> FSMState::OnUpdate(F32 dt)
{
    return nullptr;
}

Ref<FSMState> FSMState::OnCollision(const CollisionCallback::CollisionData& collision)
{
    return nullptr;
}

void FSMState::OnStateSwap(FSMState* other)
{
    ENGINE_WARN("{} {}", this->GetName(), other->GetName());
    this->OnLeave();
    other->OnEnter(this);
}

FiniteStateMachine::FiniteStateMachine(Scene& scene): m_Scene(scene)
{
}

FiniteStateMachine::~FiniteStateMachine() = default;

void FiniteStateMachine::ReadConfig(const std::string& configPath)
{
    FSMUtils::LoadAnimations(configPath, m_AnimationsMap);
}

Scene& FiniteStateMachine::GetScene()
{
    return m_Scene;
}

FiniteStateMachine::Config* FiniteStateMachine::GetConfig()
{
    return m_Config.get();
}

void FiniteStateMachine::SetDefaultCollisionResponse(CollisionCallback::SensorCallback& callback)
{
    callback = [](
       Registry* registry, const CollisionCallback::CollisionData& collisionData,
       [[maybe_unused]] const Physics::ContactInfo2D& contact)
    {
        Entity primary = collisionData.Primary;
        Entity stateHolder = NULL_ENTITY;
        if (registry->Has<Component::FSMStateComp>(primary)) stateHolder = primary;
        else stateHolder = registry->Get<Component::ParentRel>(primary).Parent;
        
        auto& stateComp = registry->Get<Component::FSMStateComp>(stateHolder);
        Ref<FSMState> newState = stateComp.CurrentState->OnCollision(collisionData);
        if (newState != nullptr)
        {
            stateComp.CurrentState->OnStateSwap(newState.get());
            stateComp.CurrentState = newState;
        }
    };
}

