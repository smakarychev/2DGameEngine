#pragma once

#include "MarioScene.h"

namespace MarioSceneUtils
{
    inline void SpawnScoreEntity(Scene& scene, U32 score, Entity spawner)
    {
        auto&& [scoreE, tf] = SceneUtils::AddDefaultEntity(scene, "Score");
        auto& registry = scene.GetRegistry();
        auto& spawnerTf = registry.Get<Component::LocalToWorldTransform2D>(spawner);
        tf.Position = spawnerTf.Position; tf.Scale = spawnerTf.Scale;
        tf.Position.y += tf.Scale.y;
        auto& scoreComp = registry.Add<Component::ScoreComponent>(scoreE);
        scoreComp.Score = score;
        auto& lifeTime = registry.Add<Component::LifeTimeComponent>(scoreE);
        lifeTime.LifeTime = lifeTime.LifeTimeLeft = 1.5;

        auto& fr = registry.Add<Component::FontRenderer>(scoreE);
        fr.Tint = {0.9f, 0.9f, 1.0f, 1.0f};
        fr.FontSize = 36.0f;
        fr.Zoom = 8;
        fr.FontRect = {
            .Min = {tf.Position.x, tf.Position.y },
            .Max = {std::numeric_limits<F32>::max(), std::numeric_limits<F32>::max()}
        };
    }

    inline Entity SpawnCoin(Scene& scene, U32 score, Entity spawner)
    {
        auto&& [coin, tf] = SceneUtils::AddDefaultEntity(scene, "Coin");
        auto& registry = scene.GetRegistry();
        auto& spawnerTf = registry.Get<Component::LocalToWorldTransform2D>(spawner);
        tf.Position = spawnerTf.Position; tf.Scale = spawnerTf.Scale;
        tf.Position.y += tf.Scale.y * 1.5f;
        registry.Add<Component::Animation>(coin);
        registry.Add<Component::SpriteRenderer>(coin);
        auto& rb = registry.Add<Component::RigidBody2D>(coin);
        rb.Flags = Physics::RigidBodyDef2D::BodyFlags(Physics::RigidBodyDef2D::RestrictRotation | Physics::RigidBodyDef2D::UseSyntheticMass);
        rb.Type = Physics::RigidBodyType2D::Dynamic;
        SceneUtils::AddDefaultPhysicalRigidBody2D(scene, coin);
        rb.PhysicsBody->SetMass(0.2f);
        SceneUtils::SynchronizePhysics(scene, coin, SceneUtils::PhysicsSynchroSetting::RBOnly);
        SpawnScoreEntity(scene, score, spawner);
        return coin;
    }
}
