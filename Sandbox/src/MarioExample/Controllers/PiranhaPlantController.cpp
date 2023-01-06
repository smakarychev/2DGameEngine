#include "PiranhaPlantController.h"

#include "../MarioScene.h"

PiranhaPlantController::PiranhaPlantController(MarioScene& scene)
    : m_Scene(scene)
{
    m_HitCallback = [](
       Registry* registry, const CollisionCallback::CollisionData& collisionData,
       [[maybe_unused]] const Physics::ContactInfo2D& contact)
    {
        Entity other = collisionData.Secondary;
        Entity parent = registry->Get<Component::ParentRel>(collisionData.Primary).Parent;
        switch (collisionData.ContactState)
        {
        case Physics::ContactListener::ContactState::Begin:
        {
            if (registry->Has<MarioPlayerTag>(other))
            {
                auto& otherState = registry->Get<Component::MarioState>(other);
                otherState.HasBeenHit = true;
            }
            break;
        }
        case Physics::ContactListener::ContactState::End: break;
        }
    };
    
    m_Scene.AddSensorCallback("PiranhaPlant", "Top sensor", m_HitCallback);
}

void PiranhaPlantController::ReadConfig(const std::string& path)
{
    YAML::Node nodes = YAML::LoadFile(path);
    m_Config.Amplitude = nodes["Amplitude"].as<F32>();
    m_Config.VerticalSpeed = nodes["VerticalSpeed"].as<F32>();
    m_Config.ApexDelay = nodes["ApexDelay"].as<F32>();
}

void PiranhaPlantController::OnUpdate(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    auto amplitude = m_Config.Amplitude;
    auto verticalSpeed = m_Config.VerticalSpeed;
    auto apexDelay = m_Config.ApexDelay;
    
    for (auto e : View<MarioPiranhaPlantTag>(registry))
    {
        auto& tf = registry.Get<Component::LocalToParentTransform2D>(e);
        tf.Position.y = Math::Min(0.0f, static_cast<F32>(amplitude * std::sin(Time::Get() * dt * verticalSpeed)));
    }
}
