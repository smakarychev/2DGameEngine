#include "QuadTreeExample.h"
#include <chrono> // TEMP
void QuadTreeExample::OnAttach()
{
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
    m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
    camera->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    camera->SetProjection(Camera::ProjectionType::Orthographic);
    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);
    
    m_MaxQuads = 1'000'000;
    m_QuadTree.Resize({ { 0.0f, 0.0f }, { 50.0f, 50.0f } });
    PopulateQuadTree();

    m_Font = Font::ReadFontFromFile("assets/fonts/Roboto-Thin.ttf");

    FrameBuffer::Spec spec;
    spec.Width = camera->GetViewportWidth(); spec.Height = camera->GetViewportHeight();
    m_FrameBuffer = FrameBuffer::Create(spec);
    RenderCommand::SetClearColor(glm::vec3(0.1f, 0.1f, 0.1f));
}

void QuadTreeExample::OnUpdate()
{
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);

    if (FrameBuffer::Spec spec = m_FrameBuffer->GetSpec();
        m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
        (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
    {
        m_FrameBuffer->Resize((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        m_CameraController->GetCamera()->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        RenderCommand::SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    }
    Render();
}

void QuadTreeExample::OnImguiUpdate()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin("Viewport");

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = { viewportSize.x, viewportSize.y };
    U64 textureID = m_FrameBuffer->GetColorBufferId();
    ImGui::Image(reinterpret_cast<void*>(textureID), viewportSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
    ImGui::PopStyleVar(1);
    ImGui::End();
}

void QuadTreeExample::PopulateQuadTree()
{
    const Rect& bounds = m_QuadTree.GetBounds();
    for (U32 i = 0; i < m_MaxQuads; i++)
    {
        ColoredQuad quad;
        quad.color = Random::Float4();
        quad.pos = glm::vec3(
            Random::Float(bounds.Center.x - bounds.HalfSize.x, bounds.Center.x + bounds.HalfSize.x),
            Random::Float(bounds.Center.y - bounds.HalfSize.y, bounds.Center.y + bounds.HalfSize.y),
            0.0f);
        quad.size = Random::Float2(0.05f, 0.125f);
        quad.vel = Random::Float2(-0.00125f, 0.00125f);
        m_QuadTree.Insert(quad, { glm::vec2(quad.pos), quad.size * 0.5f });
    }
}

void QuadTreeExample::Render()
{
    m_FrameBuffer->Bind();
    RenderCommand::ClearScreen();
    Renderer2D::BeginScene(m_CameraController->GetCamera());
    auto start = std::chrono::high_resolution_clock::now();
    auto quadsToRender = m_QuadTree.Search(GetCameraBounds());
    for (auto& quad : quadsToRender)
    {
        //Renderer2D::DrawQuad(quad.pos, quad.size, quad.color);
        Renderer2D::DrawQuad(quad->Item.pos, quad->Item.size, quad->Item.color);
    }
    for (auto& quad : quadsToRender)
    {
        glm::vec2 acceleration = Random::Float2(-0.0005f, 0.0005f);
        glm::vec2 newVel = quad->Item.vel + acceleration;
        quad->Item.vel = newVel;
        glm::vec3 newPos = quad->Item.pos + glm::vec3(quad->Item.vel, 0.0f);
        quad->Item.pos = newPos;
        m_QuadTree.Relocate(quad, { glm::vec2(quad->Item.pos),  quad->Item.size * 0.5f });
    }
    auto duration = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
    ENGINE_INFO("Frame time: {:.5f}", duration);
    Renderer2D::DrawFontFixed(*m_Font, 36.0f, 10.0f, 1600.0f, 10.0f, std::format("Visible (drawn) quads: {:d} out of {}", quadsToRender.size(), m_MaxQuads), glm::vec4(0.6f, 0.9f, 0.7f, 1.0f));
    Renderer2D::DrawFontFixed(*m_Font, 14.0f, (F32)m_FrameBuffer->GetSpec().Width - 90.0f, (F32)m_FrameBuffer->GetSpec().Width, 10.0f, std::format("Quad tree example"), glm::vec4(0.6f, 0.9f, 0.7f, 1.0f));

    Renderer2D::EndScene();

    m_FrameBuffer->Unbind();
}

Rect QuadTreeExample::GetCameraBounds()
{
    glm::vec2 min = m_CameraController->GetCamera()->ScreenToWorldPoint({ 0, m_ViewportSize.y });
    glm::vec2 max = m_CameraController->GetCamera()->ScreenToWorldPoint({ m_ViewportSize.x,  0 });
    Rect bounds;
    bounds.Center = { (max.x + min.x) / 2.0f, (max.y + min.y) / 2.0f };
    bounds.HalfSize = { (max.x - min.x) / 2.0f, (max.y - min.y) / 2.0f };
    bounds.HalfSize *= 0.9f;
    Renderer2D::DrawQuad(glm::vec3(bounds.Center, -2.0), bounds.HalfSize * 2.0f, glm::vec4(1.0f, 0.0f, 0.0f, 0.1f));
    return bounds;
}
