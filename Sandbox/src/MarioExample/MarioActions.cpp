#include "MarioActions.h"

#include "Engine/ECS/View.h"

MoveLeftAction::MoveLeftAction(Registry& registry, Status status, std::string debugName)
    : Action(status, std::move(debugName)), m_Registry(&registry)
{
}

void MoveLeftAction::Execute()
{
    for (const auto& e : View<Component::MarioInput>(*m_Registry))
    {
        auto& input = m_Registry->Get<Component::MarioInput>(e);
        switch (m_Status)
        {
        case Status::Begin:
            input.Left = true;
            input.None = false;
            break;
        case Status::End:
            input.Left = false;
            break;
        }
    }
}

MoveRightAction::MoveRightAction(Registry& registry, Status status, std::string debugName)
    : Action(status, std::move(debugName)), m_Registry(&registry)
{
}

void MoveRightAction::Execute()
{
    for (const auto& e : View<Component::MarioInput>(*m_Registry))
    {
        auto& input = m_Registry->Get<Component::MarioInput>(e);
        switch (m_Status)
        {
        case Status::Begin:
            input.Right = true;
            input.None = false;
            break;
        case Status::End:
            input.Right = false;
            break;
        }
    }
}

JumpAction::JumpAction(Registry& registry, Status status, std::string debugName)
    : Action(status, std::move(debugName)), m_Registry(&registry)
{
}

void JumpAction::Execute()
{
    for (const auto& e : View<Component::MarioInput>(*m_Registry))
    {
        auto& input = m_Registry->Get<Component::MarioInput>(e);
        switch (m_Status)
        {
        case Status::Begin:
            input.Jump = true;
            input.None = false;
            break;
        case Status::End:
            input.Jump = false;
            break;
        }
    }
}
