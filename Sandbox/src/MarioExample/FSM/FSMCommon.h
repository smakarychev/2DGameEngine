#pragma once

#include "Engine.h"

using namespace Engine;

class FSMUtils
{
public:

    static I32 GetHorizontalMoveDir()
    {
        I32 move = 0;
        if (Input::GetKey(Key::A)) move = -1;
        if (Input::GetKey(Key::D)) move = 1;
        return move;
    }

    static void FlipSpriteBasedOnMoveDir(Scene& scene, Entity e, I32 moveDir)
    {
        auto& registry = scene.GetRegistry();
        auto& spr = registry.Get<Component::SpriteRenderer>(e);
        if (moveDir == 1) spr.FlipX = false;
        else if (moveDir == -1) spr.FlipX = true;
    }

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

    static void LoadAnimations(const std::string& path, std::unordered_map<std::string, Ref<SpriteAnimation>>& animationsMap)
    {
        YAML::Node nodes = YAML::LoadFile(path);
        auto animations = nodes["Animations"];
        for (auto animation : animations)
        {
            auto name = animation["Name"].as<std::string>();
            auto sprites = Texture::LoadTextureFromFile(animation["Sprites"].as<std::string>());
            auto startPoint = animation["StartPoint"].as<glm::uvec2>();
            auto spriteSize = animation["SpriteSize"].as<glm::uvec2>();
            auto frameCount = animation["FrameCount"].as<U32>();
            auto fpsSpeed = animation["FpsSpeed"].as<U32>();
            auto maxDuration = animation["MaxDuration"].as<U32>();

            animationsMap.emplace(name, CreateRef<SpriteAnimation>(sprites.get(), startPoint, spriteSize, frameCount, fpsSpeed, maxDuration));
        }
    }
};