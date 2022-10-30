#include "ParticlePhysicsExample.h"

void ParticlePhysicsExample::OnAttach()
{
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
    m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
    camera->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    camera->SetProjection(Camera::ProjectionType::Orthographic);
    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);

    m_Font = Font::ReadFontFromFile("assets/fonts/Roboto-Thin.ttf");

    FrameBuffer::Spec spec;
    spec.Width = camera->GetViewportWidth(); spec.Height = camera->GetViewportHeight();
    m_FrameBuffer = FrameBuffer::Create(spec);
    RenderCommand::SetClearColor(glm::vec3(0.1f, 0.1f, 0.1f));
    //m_World.AddForce(CreateRef<ParticleDrag>(ParticleDrag(0.1f, 0.2f)));

    auto* nw = &m_World.CreateParticle(glm::vec3{ -2.0f,  4.0f, 0.0f }); // 0
    auto* ne = &m_World.CreateParticle(glm::vec3{  2.0f,  4.0f, 0.0f }); // 1
    auto* sw = &m_World.CreateParticle(glm::vec3{ -2.0f, -1.0f, 0.0f }); // 2
    auto* se = &m_World.CreateParticle(glm::vec3{  2.0f, -1.0f, 0.0f }); // 3
    nw->SetInverseMass(0.0f); ne->SetInverseMass(0.0f); sw->SetInverseMass(0.0f); se->SetInverseMass(0.0f);

    auto* moverA = &m_World.CreateParticle(glm::vec3{ 0.0f,  3.0f, 0.0f }); // 4
    auto* moverB = &m_World.CreateParticle(glm::vec3{ 0.0f,  1.0f, 0.0f }); // 5

    m_World.AddForce(CreateRef<ParticleSpring>(ParticleSpring(nw, 1.0f, 0.5f)), *moverA);
    m_World.AddForce(CreateRef<ParticleSpring>(ParticleSpring(ne, 1.0f, 0.5f)), *moverA);

    m_World.AddForce(CreateRef<ParticleSpring>(ParticleSpring(sw, 10.0f, 0.5f)), *moverB);
    m_World.AddForce(CreateRef<ParticleSpring>(ParticleSpring(se, 1.0f, 0.5f)), *moverB);

    m_World.CreateParticleRod(*moverA, *moverB, 3.0f);
    //m_World.CreateParticleCable(*moverA, *moverB, 3.0f, 0.8f);
}

void ParticlePhysicsExample::OnUpdate()
{
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);
    ValidateViewport();
    m_World.Update(dt);
    Render();
}

void ParticlePhysicsExample::OnImguiUpdate()
{
    m_ViewportSize = ImguiMainViewport(*m_FrameBuffer);
}

void ParticlePhysicsExample::ValidateViewport()
{
    if (FrameBuffer::Spec spec = m_FrameBuffer->GetSpec();
        m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
        (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
    {
        m_FrameBuffer->Resize((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        m_CameraController->GetCamera()->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        RenderCommand::SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    }
}

void ParticlePhysicsExample::Render()
{
    m_FrameBuffer->Bind();
    RenderCommand::ClearScreen();
    Renderer2D::BeginScene(m_CameraController->GetCamera());
    // Render particles.
    for (auto& particle : m_World.GetParticles())
    {
        Component::Transform2D transform;
        transform.Position = particle->GetPosition();
        transform.Scale = {0.2f, 0.2f};
        Component::SpriteRenderer sp;
        
        Renderer2D::DrawQuad(transform, sp);
    }
    Renderer2D::DrawLine(m_World.GetParticles()[4]->GetPosition(), m_World.GetParticles()[0]->GetPosition(), glm::vec4{ 0.2f, 0.8f, 0.2f, 1.0f });
    Renderer2D::DrawLine(m_World.GetParticles()[4]->GetPosition(), m_World.GetParticles()[1]->GetPosition(), glm::vec4{ 0.2f, 0.8f, 0.2f, 1.0f });
    Renderer2D::DrawLine(m_World.GetParticles()[2]->GetPosition(), m_World.GetParticles()[5]->GetPosition(), glm::vec4{ 0.2f, 0.8f, 0.2f, 1.0f });
    Renderer2D::DrawLine(m_World.GetParticles()[3]->GetPosition(), m_World.GetParticles()[5]->GetPosition(), glm::vec4{ 0.2f, 0.8f, 0.2f, 1.0f });

    Renderer2D::DrawLine(m_World.GetParticles()[4]->GetPosition(), m_World.GetParticles()[5]->GetPosition(), glm::vec4{ 0.2f, 0.2f, 0.8f, 1.0f });
    Component::FontRenderer fr;
    fr.Font = m_Font.get();
    fr.Tint = {0.6f, 0.9f, 0.7f, 1.0f};
    fr.FontSize = 14.0f;
    fr.FontRect = {.Min = {(F32)m_FrameBuffer->GetSpec().Width - 130.0f, 10.0f}, .Max = {(F32)m_FrameBuffer->GetSpec().Width, 10.0f}};
    Renderer2D::DrawFontFixed(fr, "particle physics example");

    Renderer2D::EndScene();

    m_FrameBuffer->Unbind();
}

CRect ParticlePhysicsExample::GetCameraBounds()
{
    glm::vec2 min = m_CameraController->GetCamera()->ScreenToWorldPoint({ 0, m_ViewportSize.y });
    glm::vec2 max = m_CameraController->GetCamera()->ScreenToWorldPoint({ m_ViewportSize.x,  0 });
    CRect bounds;
    bounds.Center = { (max.x + min.x) / 2.0f, (max.y + min.y) / 2.0f };
    bounds.HalfSize = { (max.x - min.x) / 2.0f, (max.y - min.y) / 2.0f };
    return bounds;
}
