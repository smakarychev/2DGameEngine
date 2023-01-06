#include "GoombaController.h"

#include "../MarioScene.h"

GoombaController::GoombaController(MarioScene& scene)
    : m_Scene(scene)
{
    // Create hit callback.
    m_HitCallback = [](
       Registry* registry, const CollisionCallback::CollisionData& collisionData,
       [[maybe_unused]] const Physics::ContactInfo2D& contact)
    {
        Entity other = collisionData.Secondary;
        Entity parent = registry->Get<Component::ParentRel>(collisionData.Primary).Parent;
        switch (collisionData.ContactState)
        {
        case Physics::ContactListener::ContactState::Begin:
        {
            auto& state = registry->Get<Component::GoombaState>(parent);
            if (registry->Has<MarioLevelTag>(other) || registry->Has<MarioEnemyTag>(other)) state.HasHitWall = true;
            else if (registry->Has<MarioPlayerTag>(other) && !registry->Get<Component::GoombaState>(parent).HasBeenHitByPlayer)
            {
                auto& otherState = registry->Get<Component::MarioState>(other);
                otherState.HasBeenHit = true;
            }
            break;
        }
        case Physics::ContactListener::ContactState::End: break;
        }
    };
    
    m_Scene.AddSensorCallback("Goomba", "Left sensor", m_HitCallback);
    m_Scene.AddSensorCallback("Goomba", "Right sensor", m_HitCallback);
}

void GoombaController::OnInit()
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<Component::GoombaState>(registry))
    {
        auto& state = registry.Get<Component::GoombaState>(e);
        state.IsMovingRight = true;
    }
}

void GoombaController::ReadConfig(const std::string& path)
{
    YAML::Node nodes = YAML::LoadFile(path);
    m_Config.HorizontalSpeed = nodes["HorizontalSpeed"].as<F32>();
    m_Config.DeathTime = nodes["DeathTime"].as<F64>();
}

void GoombaController::OnUpdate(F32 dt)
{
    UpdateState();
    UpdatePosition(dt);
}

void GoombaController::UpdateState()
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<Component::GoombaState>(registry))
    {
        auto& state = registry.Get<Component::GoombaState>(e);

        if (state.HasBeenHitByPlayer)
        {
            OnGoombaKill(e); continue;
        }
        
        if (state.HasHitWall)
        {
            std::swap(state.IsMovingLeft, state.IsMovingRight);
            state.HasHitWall = false;
        }
    }
}

void GoombaController::UpdatePosition(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    auto horizontalSpeed = m_Config.HorizontalSpeed;
    for (auto e : View<Component::GoombaState>(registry))
    {
        if (registry.Has<Component::KillComponent>(e)) continue;

        auto& state = registry.Get<Component::GoombaState>(e);
        auto& rb = registry.Get<Component::RigidBody2D>(e);
        auto& vel = rb.PhysicsBody->GetLinearVelocity();

        vel.x = state.IsMovingRight ? horizontalSpeed : -horizontalSpeed;
        if (state.HasBeenHitByPlayer) vel.x = 0.0f;
    }
}

void GoombaController::OnGoombaKill(Entity goomba)
{
    auto& registry = m_Scene.GetRegistry();

    if (!registry.Has<Component::KillComponent>(goomba))
    {
        auto& rb = registry.Get<Component::RigidBody2D>(goomba);
        glm::vec2& vel = rb.PhysicsBody->GetLinearVelocity();
        vel = { 0.0f, 0.0f };
        registry.Remove<Component::BoxCollider2D>(goomba);
        registry.Remove<Component::RigidBody2D>(goomba);
        auto& killComponent = registry.Add<Component::KillComponent>(goomba);
        killComponent.LifeTime = m_Config.DeathTime;
    }
}



