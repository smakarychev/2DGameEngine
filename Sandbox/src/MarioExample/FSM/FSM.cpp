#include "FSM.h"

#include "../Animators/AnimatorsCommon.h"

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
    AnimatorUtils::LoadAnimations(configPath, m_AnimationsMap);
}

Scene& FiniteStateMachine::GetScene()
{
    return m_Scene;
}

FiniteStateMachine::Config* FiniteStateMachine::GetConfig()
{
    return m_Config.get();
}

