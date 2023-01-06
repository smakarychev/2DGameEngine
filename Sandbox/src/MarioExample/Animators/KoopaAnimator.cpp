#include "KoopaAnimator.h"

#include "AnimatorsCommon.h"

KoopaAnimator::KoopaAnimator(Scene& scene)
    : m_Scene(scene)
{
}

void KoopaAnimator::LoadAnimations(const std::string& path)
{
    AnimatorUtils::LoadAnimations(path, m_AnimationsMap);
}

void KoopaAnimator::OnUpdate()
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<Component::Animation, Component::KoopaState>(registry))
    {
        auto& animation = registry.Get<Component::Animation>(e);
        const auto& state = registry.Get<Component::KoopaState>(e);
        if (!state.HitByKoopa)
        {
            if (state.HitByPlayerCount == 0)
            {
                if (state.IsMovingLeft && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["wLeft"]->GetUUID())
                {
                    animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["wLeft"]);
                }
                if (state.IsMovingRight && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["wRight"]->GetUUID())
                {
                    animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["wRight"]);
                }
            }
            else if (state.HitByPlayerCount == 1)
            {
                if (animation.SpriteAnimation->GetUUID() != m_AnimationsMap["hOnce"]->GetUUID())
                {
                    animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["hOnce"]);
                }
            }
            else if (state.HitByPlayerCount == 2)
            {
                if (animation.SpriteAnimation->GetUUID() != m_AnimationsMap["hTwice"]->GetUUID())
                {
                    animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["hTwice"]);
                }
            }
        }
        else
        {
            if (animation.SpriteAnimation->GetUUID() != m_AnimationsMap["hit"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["hit"]);
            }
        }        
    }
}
