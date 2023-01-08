#pragma once

#include "Engine.h"

#include "../MarioContactListener.h"

using namespace Engine::Types;
using namespace Engine;

class FSMState
{
public:
    FSMState(const std::string& name, Entity e, FiniteStateMachine& fsm);
    virtual ~FSMState();
    virtual void OnEnter([[maybe_unused]] FSMState* previousState);
    virtual void OnLeave();
    virtual Ref<FSMState> OnUpdate(F32 dt);
    virtual Ref<FSMState> OnCollision(const CollisionCallback::CollisionData& collision);
    const std::string& GetName() const { return m_Name; }
    std::string& GetName() { return m_Name; }
    void OnStateSwap(FSMState* other);
protected:
    std::string m_Name;
    Entity m_Entity{NULL_ENTITY};
    FiniteStateMachine& m_FSM;
};

class FiniteStateMachine
{
public:
    using AnimationsMap = std::unordered_map<std::string, Ref<SpriteAnimation>>;
    class Config {};
public:
    FiniteStateMachine(Scene& scene);
    virtual ~FiniteStateMachine();
    virtual void ReadConfig(const std::string& configPath);
    virtual void OnUpdate(F32 dt) = 0;
    virtual void RegisterEntity(Entity e) = 0;
    Scene& GetScene();
    Config* GetConfig();
    AnimationsMap& GetAnimationsMap() { return m_AnimationsMap; }
    static void SetDefaultCollisionResponse(CollisionCallback::SensorCallback& callback);
    template <typename T>
    T& As();
protected:
    Scene& m_Scene;
    Ref<Config> m_Config{};
    AnimationsMap m_AnimationsMap;
};

template <typename T>
T& FiniteStateMachine::As()
{
    return *static_cast<T*>(this);
}
