#pragma once

#include "Engine.h"

using namespace Engine;
using namespace Engine::Types;

class MoveLeftAction : public Action
{
public:
    MoveLeftAction(Registry& registry, Status status = Status::Begin, std::string debugName = "MoveLeft");
    void Execute() override;
private:
    Registry* m_Registry = nullptr;
};

class MoveRightAction : public Action
{
public:
    MoveRightAction(Registry& registry, Status status = Status::Begin, std::string debugName = "MoveRight");
    void Execute() override;
private:
    Registry* m_Registry = nullptr;
};

class JumpAction : public Action
{
public:
    JumpAction(Registry& registry, Status status = Status::Begin, std::string debugName = "Jump");
    void Execute() override;
private:
    Registry* m_Registry = nullptr;
};
