#include "PlayerController.h"

#include "ControllersCommon.h"
#include "../MarioScene.h"

PlayerController::PlayerController(MarioScene& scene)
    : m_Scene(scene)
{
    // What is SOLID once again?
    
    // Create jump callback.
    m_FootCallback = [](
       Registry* registry, const CollisionCallback::CollisionData& collisionData,
       [[maybe_unused]] const Physics::ContactInfo2D& contact)
    {
        auto& collisionCallback = registry->Get<CollisionCallback>(collisionData.Primary);
        Entity other = collisionData.Secondary;
        Entity parent = registry->Get<Component::ParentRel>(collisionData.Primary).Parent;
        switch (collisionData.ContactState)
        {
        case Physics::ContactListener::ContactState::Begin:
        {
            if (registry->Has<MarioLevelTag>(other))
            {
                collisionCallback.CollisionCount++;
                registry->Get<Component::MarioState>(parent).IsGrounded = true;
            }
            else if (!registry->Get<Component::MarioState>(parent).HasBeenHit)
            {
                if (registry->Has<MarioGoombaTag>(other))
                {
                    registry->Get<Component::MarioState>(parent).HasHitEnemy = true;
                    auto& otherState = registry->Get<Component::GoombaState>(other);
                    otherState.HasBeenHitByPlayer = true;
                }
                else if (registry->Has<MarioKoopaTag>(other))
                {
                    registry->Get<Component::MarioState>(parent).HasHitEnemy = true;
                    auto& otherState = registry->Get<Component::KoopaState>(other);
                    if (otherState.HitByPlayerCount < 2) otherState.HitByPlayerCount++;
                }
            }
            break;
        }
        case Physics::ContactListener::ContactState::End:
        {
            if (registry->Has<MarioLevelTag>(other))
            {
                collisionCallback.CollisionCount--;
                if (collisionCallback.CollisionCount == 0) registry->Get<Component::MarioState>(parent).IsGrounded = false;
            }
            
            break; 
        }
        }
    };
    m_Scene.AddSensorCallback("Mario", "Bottom sensor", m_FootCallback);
}

void PlayerController::OnUpdate(F32 dt)
{
    GatherInput();
    UpdatePosition(dt);
}

void PlayerController::ReadConfig(const std::string& path)
{
    YAML::Node nodes = YAML::LoadFile(path);
    m_Config.JumpImpulse = nodes["JumpImpulse"].as<F32>();
    m_Config.MaxHorizontalSpeed = nodes["MaxHorizontalSpeed"].as<F32>();
    m_Config.MaxFallSpeed = nodes["MaxFallSpeed"].as<F32>();
    m_Config.VerticalVelocityDrop = nodes["VerticalVelocityDrop"].as<F32>();
    m_Config.HorizontalVelocityDrop = nodes["HorizontalVelocityDrop"].as<F32>();
    m_Config.CoyoteTime = nodes["CoyoteTime"].as<F64>();
    m_Config.JumpBuffer = nodes["JumpBuffer"].as<F64>();
    m_Config.DeathTime = nodes["DeathTime"].as<F64>();
}

void PlayerController::GatherInput()
{
    auto& registy = m_Scene.GetRegistry();
    for (auto e : View<Component::MarioInput>(registy))
    {
        auto& input = registy.Get<Component::MarioInput>(e);
        input.JumpDown = Input::GetKeyDown(Key::Space);
        input.JumpUp = Input::GetKeyUp(Key::Space);
        input.Left = Input::GetKey(Key::A);
        input.Right = Input::GetKey(Key::D);
        input.None = !(input.JumpDown || input.Left || input.Right);
    }
}

void PlayerController::UpdatePosition(F32 dt)
{
    auto& registy = m_Scene.GetRegistry();
    auto maxHorizontalSpeed = m_Config.MaxHorizontalSpeed;
    auto maxFallSpeed = m_Config.MaxFallSpeed;
    auto jumpImpulse = m_Config.JumpImpulse;
    auto vertVelDrop = m_Config.VerticalVelocityDrop;
    auto horVelDrop = m_Config.HorizontalVelocityDrop;
    auto coyteTime = m_Config.CoyoteTime;
    auto jumpBuffer = m_Config.JumpBuffer;
    for (auto e : View<Component::MarioInput, Component::RigidBody2D>(registy))
    {
        // Update player position.
        auto& state = registy.Get<Component::MarioState>(e);

        if (state.HasBeenHit)
        {
            OnPlayerKill(e); continue;
        }
        
        auto& input = registy.Get<Component::MarioInput>(e);
        auto& rb = registy.Get<Component::RigidBody2D>(e);
        glm::vec2& vel = rb.PhysicsBody->GetLinearVelocity();

        if (state.IsGrounded) m_CoyoteTimeCounter = coyteTime;
        else m_CoyoteTimeCounter -= static_cast<F64>(dt);
        if (input.JumpDown) m_JumpBufferTimeCounter = jumpBuffer;
        else m_JumpBufferTimeCounter -= static_cast<F64>(dt);

        if (m_CoyoteTimeCounter > 0.0 && m_JumpBufferTimeCounter > 0.0)
        {
            vel.y = 0.0f;
            rb.PhysicsBody->AddForce(glm::vec2{0.0f, jumpImpulse}, Physics::ForceMode::Impulse);
            m_JumpBufferTimeCounter = 0.0;
        }
        
        else if (input.JumpUp && vel.y > 0.0f)
        {
            if (vel.y > 0.0f) vel.y *= vertVelDrop;
            m_CoyoteTimeCounter = 0.0;
        }

        if (state.HasHitEnemy)
        {
            rb.PhysicsBody->AddForce(glm::vec2{vel.x > 0 ? jumpImpulse : -jumpImpulse, jumpImpulse}, Physics::ForceMode::Impulse);
            state.HasHitEnemy = false;
        }
        
        if (input.None) vel.x *= horVelDrop;
        if (input.Left) rb.PhysicsBody->AddForce(glm::vec2{-90.0f, 0.0f});
        if (input.Right) rb.PhysicsBody->AddForce(glm::vec2{90.0f, 0.0f});
        vel.x = Math::Clamp(vel.x, -maxHorizontalSpeed, maxHorizontalSpeed);
        vel.y = Math::Clamp(vel.y, maxFallSpeed, std::numeric_limits<F32>::max());
            
        rb.PhysicsBody->SetLinearVelocity(vel);
    }
}

void PlayerController::OnPlayerKill(Entity player)
{
    auto& registry = m_Scene.GetRegistry();

    if (!registry.Has<Component::KillComponent>(player))
    {
        auto jumpImpulse = m_Config.JumpImpulse;
        ControllerUtils::OnDeathImpulse(player, {0.0f, jumpImpulse}, m_Scene);
        auto& killComponent = registry.Add<Component::KillComponent>(player);
        killComponent.LifeTime = m_Config.DeathTime;
    }
}
