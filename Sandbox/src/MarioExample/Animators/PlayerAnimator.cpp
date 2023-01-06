#include "PlayerAnimator.h"

#include "AnimatorsCommon.h"

PlayerAnimator::PlayerAnimator(Scene& scene)
    : m_Scene(scene)
{
}

void PlayerAnimator::LoadAnimations(const std::string& path)
{
    AnimatorUtils::LoadAnimations(path, m_AnimationsMap);
}

void PlayerAnimator::OnUpdate()
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<Component::Animation, Component::MarioState>(registry))
    {
        auto& animation = registry.Get<Component::Animation>(e);
        const auto& state = registry.Get<Component::MarioState>(e);

        if (!state.IsGrounded)
        {
            if (state.IsFacingLeft && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["jLeft"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["jLeft"]);
            }
            else if (state.IsFacingRight && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["jRight"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["jRight"]);
            }
        }
        else
        {
            if (state.IsFacingLeft && !state.IsMovingLeft) animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["iLeft"]);
            else if (state.IsFacingRight && !state.IsMovingRight) animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["iRight"]);
        }
        if (state.IsMovingLeft)
        {
            if (state.IsGrounded && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["wLeft"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["wLeft"]);
            }
        }
        else if (state.IsMovingRight)
        {
            if (state.IsGrounded && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["wRight"]->GetUUID())
            {
                animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["wRight"]);
            }
        }
        if (state.HasBeenHit && animation.SpriteAnimation->GetUUID() != m_AnimationsMap["hit"]->GetUUID())
        {
            animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["hit"]);
        }
    }
}
