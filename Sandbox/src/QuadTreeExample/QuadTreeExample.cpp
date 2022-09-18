#include "QuadTreeExample.h"
#include <chrono> // TEMP
void QuadTreeExample::OnAttach()
{
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
    m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
    camera->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    camera->SetProjection(Camera::ProjectionType::Orthographic);
    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);
    
    m_MaxQuads = 20'000;
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
    m_ViewportSize = ImguiMainViewport(*m_FrameBuffer);
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
    auto quadsToRender = m_QuadTree.Search(GetCameraBounds());
    auto start = std::chrono::high_resolution_clock::now();

    for (auto& quad : quadsToRender)
    {
        //Renderer2D::DrawQuad(quad.pos, quad.size, quad.color);
        Renderer2D::DrawQuad({ .Position{ quad->Item.pos },
            .Scale{ quad->Item.size },
            .Color{ quad->Item.color } }
        );
    }
    for (U32 i = 0; i < m_QuadTree.GetItems().size(); i++)
    {
        auto& quad = m_QuadTree.GetItems()[i];
        glm::vec2 acceleration = Random::Float2(-0.005f, 0.005f);
        glm::vec2 newVel = quad.Item.vel + acceleration;
        quad.Item.vel = newVel;
        glm::vec3 newPos = quad.Item.pos + glm::vec3(quad.Item.vel, 0.0f);
        quad.Item.pos = newPos;

        if (quad.Item.pos.x < m_QuadTree.GetBounds().Center.x - m_QuadTree.GetBounds().HalfSize.x || quad.Item.pos.x >  m_QuadTree.GetBounds().Center.x + m_QuadTree.GetBounds().HalfSize.x)
            quad.Item.vel.x *= -1.0f;
        if (quad.Item.pos.y < m_QuadTree.GetBounds().Center.y - m_QuadTree.GetBounds().HalfSize.y || quad.Item.pos.y >  m_QuadTree.GetBounds().Center.y + m_QuadTree.GetBounds().HalfSize.y)
            quad.Item.vel.y *= -1.0f;

        m_QuadTree.Relocate(m_QuadTree.GetItems().begin() + i, {glm::vec2(quad.Item.pos),  quad.Item.size * 0.5f});
    }
    for (auto& quad : quadsToRender)
    {
        auto overlapping = m_QuadTree.Search({ quad->Item.pos, quad->Item.size * 0.5f });
        if (overlapping.size() > 1)
        {
            quad->Item.vel *= -1.0f;
        }
    }
    auto duration = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
    ENGINE_INFO("Frame time: {:.5f} ({:.0f} fps)", duration, 1.0 / duration);
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
    Renderer2D::DrawQuad({ .Position{ glm::vec3(bounds.Center, -2.0) },
        .Scale{ bounds.HalfSize * 2.0f },
        .Color{ glm::vec4(1.0f, 0.0f, 0.0f, 0.1f) } }
    );
    return bounds;
}
