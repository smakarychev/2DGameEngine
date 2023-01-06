#include "KoopaController.h"

#include "ControllersCommon.h"
#include "../MarioScene.h"

KoopaController::KoopaController(MarioScene& scene)
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
            auto& state = registry->Get<Component::KoopaState>(parent);
            if (registry->Has<MarioKoopaTag>(other) && registry->Get<Component::KoopaState>(other).HitByPlayerCount == 2)
            {
                state.HitByKoopa = true;
            }
            else if (registry->Has<MarioLevelTag>(other) || registry->Has<MarioEnemyTag>(other))
            {
                state.HasHitWall = true;
            }
            else if (registry->Has<MarioPlayerTag>(other) && registry->Get<Component::KoopaState>(parent).HitByPlayerCount != 1)
            {
                auto& otherState = registry->Get<Component::MarioState>(other);
                otherState.HasBeenHit = true;
            }
            break;
        }
        case Physics::ContactListener::ContactState::End: break;
        }
    };

    m_Scene.AddSensorCallback("Koopa", "Left sensor", m_HitCallback);
    m_Scene.AddSensorCallback("Koopa", "Right sensor", m_HitCallback);
}

void KoopaController::OnInit()
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<Component::KoopaState>(registry))
    {
        auto& state = registry.Get<Component::KoopaState>(e);
        state.IsMovingRight = true;
    }
}

void KoopaController::ReadConfig(const std::string& path)
{
    YAML::Node nodes = YAML::LoadFile(path);
    m_Config.HorizontalSpeed = nodes["HorizontalSpeed"].as<F32>();
    m_Config.TurnedHorizontalSpeed = nodes["TurnedHorizontalSpeed"].as<F32>();
    m_Config.DeathTime = nodes["DeathTime"].as<F64>();
    m_Config.DeathImpulse = nodes["DeathImpulse"].as<F32>();
}

void KoopaController::OnUpdate(F32 dt)
{
    UpdateState();
    UpdatePosition(dt);
}

void KoopaController::UpdateState()
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<Component::KoopaState>(registry))
    {
        auto& state = registry.Get<Component::KoopaState>(e);

        if (state.HitByKoopa)
        {
            OnKoopaKill(e); continue;
        }
        
        if (state.HasHitWall)
        {
            std::swap(state.IsMovingLeft, state.IsMovingRight);
            state.HasHitWall = false;
        }
    }
}

void KoopaController::UpdatePosition(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    auto horizontalSpeed = m_Config.HorizontalSpeed;
    auto turnedHorizontalSpeed = m_Config.TurnedHorizontalSpeed;
    for (auto e : View<Component::KoopaState>(registry))
    {
        auto& state = registry.Get<Component::KoopaState>(e);
        auto& rb = registry.Get<Component::RigidBody2D>(e);
        auto& vel = rb.PhysicsBody->GetLinearVelocity();

        switch (state.HitByPlayerCount)
        {
        case 0: vel.x = state.IsMovingRight ? horizontalSpeed : -horizontalSpeed; break;
        case 1: vel.x = 0.0f; break;            
        // 2
        default: vel.x = state.IsMovingRight ? turnedHorizontalSpeed : -turnedHorizontalSpeed; break;
        }
    }
}

void KoopaController::OnKoopaKill(Entity koopa)
{
    auto& registry = m_Scene.GetRegistry();

    if (!registry.Has<Component::KillComponent>(koopa))
    {
        auto deathImpulse = m_Config.DeathImpulse;
        ControllerUtils::OnDeathImpulse(koopa, {0.0f, deathImpulse}, m_Scene);
        auto& killComponent = registry.Add<Component::KillComponent>(koopa);
        killComponent.LifeTime = m_Config.DeathTime;
    }
}
