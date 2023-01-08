#include "MarioCameraController.h"

MarioCameraController::MarioCameraController(Ref<Camera> camera, Scene& scene)
    : CameraController(std::move(camera)), m_Scene(scene)
{
    m_Type = ControllerType::Custom;
    m_Camera->SetZoom(8);
}

void MarioCameraController::ReadConfig(const std::string& path)
{
    YAML::Node nodes = YAML::LoadFile(path);
    m_Config.FollowSpeed = nodes["FollowSpeed"].as<F32>();
    m_Config.DistanceThresholdX = nodes["DistanceThresholdX"].as<F32>();
    m_Config.DistanceThresholdY = nodes["DistanceThresholdY"].as<F32>();
    m_Config.TargetOffset = nodes["TargetOffset"].as<glm::vec2>();
}

void MarioCameraController::SetTarget(Entity target)
{
    m_Target = target;
}

void MarioCameraController::OnUpdate(F32 dt)
{
    auto& registry = m_Scene.GetRegistry();
    if (m_Target == NULL_ENTITY || !registry.Has<Component::LocalToWorldTransform2D>(m_Target)) return;
    if (registry.Has<Component::LifeTimeComponent>(m_Target)) return;

    auto followSpeed = m_Config.FollowSpeed;
    auto distanceThresholdX = m_Config.DistanceThresholdX;
    auto distanceThresholdY = m_Config.DistanceThresholdY;
    auto targetOffset = m_Config.TargetOffset;
    
    auto& targetTf = registry.Get<Component::LocalToWorldTransform2D>(m_Target);

    glm::vec2 cameraPos = m_Camera->GetPosition();
    glm::vec2 diff = (targetTf.Position + targetOffset) - cameraPos;
    // Camera follows only to the right
    if (diff.x > 0 && diff.x > distanceThresholdX) cameraPos.x += followSpeed * diff.x * dt;
    if (Math::Abs(diff.y) > distanceThresholdY) cameraPos.y += followSpeed * diff.y * dt;
    F32 pixelSize = m_Camera->ScreenToWorldPoint({1.0f, 1.0f}).x - m_Camera->ScreenToWorldPoint({0.0f, 0.0f}).x;
    cameraPos = Math::Align(cameraPos, {pixelSize, pixelSize});
    m_Camera->SetPosition(glm::vec3{ cameraPos, m_Camera->GetPosition().z });
}

bool MarioCameraController::OnEvent(Event& event)
{
    return false;
}
