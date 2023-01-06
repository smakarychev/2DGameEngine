#include "GoombaAnimator.h"

#include "AnimatorsCommon.h"

GoombaAnimator::GoombaAnimator(Scene& scene)
    : m_Scene(scene)
{
}

void GoombaAnimator::LoadAnimations(const std::string& path)
{
    AnimatorUtils::LoadAnimations(path, m_AnimationsMap);
}

void GoombaAnimator::OnUpdate()
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<Component::Animation, Component::GoombaState>(registry))
    {
        auto& animation = registry.Get<Component::Animation>(e);
        const auto& state = registry.Get<Component::GoombaState>(e);
        if (!state.HasBeenHitByPlayer && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["walk"]->GetUUID())
        {
            animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["walk"]);
        }
        if (state.HasBeenHitByPlayer && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["hit"]->GetUUID())
        {
            animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["hit"]);
        }
    }
}
