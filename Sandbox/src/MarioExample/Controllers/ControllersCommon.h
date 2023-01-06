#pragma once

#include "Engine.h"

using namespace Engine::Types;
using namespace Engine;

struct ControllerUtils
{
    // Once again not the best name.
    static void OnDeathImpulse(Entity e, const glm::vec2& impulse, Scene& scene)
    {
        auto& registry = scene.GetRegistry();
        auto& col = registry.Get<Component::BoxCollider2D>(e);
        col.IsSensor = true;
        auto& rb = registry.Get<Component::RigidBody2D>(e);
        glm::vec2& vel = rb.PhysicsBody->GetLinearVelocity();
        vel = { 0.0f, 0.0f };
        rb.PhysicsBody->AddForce(glm::vec2{impulse.x, impulse.y}, Physics::ForceMode::Impulse);
    }

    static void KillSystem(F32 dt, Scene& scene)
    {
        auto& registry = scene.GetRegistry();
        for (auto e : View<Component::KillComponent>(registry))
        {
            auto& killComponent = registry.Get<Component::KillComponent>(e);
            killComponent.LifeTime -= dt;
            if (killComponent.LifeTime < 0.0) SceneUtils::DeleteEntity(scene, e);
        }
    }
};