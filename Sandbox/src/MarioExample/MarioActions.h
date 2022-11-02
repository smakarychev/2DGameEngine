#pragma once

#include "Engine.h"

using namespace Engine;
using namespace Engine::Types;

class MoveLeftAction : public Action
{
public:
    MoveLeftAction(EntityManager* manager, Status status, std::string debugName);
    void Execute() override;
private:
    EntityManager* m_EntityManager = nullptr;
};
