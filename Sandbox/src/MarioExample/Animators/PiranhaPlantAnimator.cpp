#include "PiranhaPlantAnimator.h"

#include "AnimatorsCommon.h"
#include "../MarioTags.h"

PiranhaPlantAnimator::PiranhaPlantAnimator(Scene& scene)
    : m_Scene(scene)
{
}

void PiranhaPlantAnimator::LoadAnimations(const std::string& path)
{
    AnimatorUtils::LoadAnimations(path, m_AnimationsMap);
}

void PiranhaPlantAnimator::OnUpdate()
{
    auto& registry = m_Scene.GetRegistry();
    for (auto e : View<Component::Animation, MarioPiranhaPlantTag>(registry))
    {
        auto& animation = registry.Get<Component::Animation>(e);
        if (animation.SpriteAnimation->GetUUID() != m_AnimationsMap["bite"]->GetUUID())
        {
            animation.SpriteAnimation = CreateRef<SpriteAnimation>(*m_AnimationsMap["bite"]);
        }
    }
}
