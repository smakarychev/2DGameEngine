#include "MoveLeftAction.h"

MoveLeftAction::MoveLeftAction(EntityManager* manager, Status status, std::string debugName)
    : Action(status, std::move(debugName))
{
}
