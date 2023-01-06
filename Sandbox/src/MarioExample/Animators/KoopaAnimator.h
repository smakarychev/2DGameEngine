#pragma once

#include "Engine.h"

using namespace Engine;

class KoopaAnimator
{
public:
    KoopaAnimator(Scene& scene);
    void LoadAnimations(const std::string& path);
    void OnUpdate();
private:
    Scene& m_Scene;
    std::unordered_map<std::string, Ref<SpriteAnimation>> m_AnimationsMap;
};
