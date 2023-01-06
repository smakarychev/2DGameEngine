#pragma once

#include "Engine.h"

using namespace Engine::Types;
using namespace Engine;

struct AnimatorUtils
{
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