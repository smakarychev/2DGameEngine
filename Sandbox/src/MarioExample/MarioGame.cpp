#include "MarioGame.h"

#include "MarioScene.h"

void MarioGame::OnAttach()
{
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);
    m_ViewportSize = { Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() };
    camera->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    camera->SetProjection(Camera::ProjectionType::Orthographic);
    camera->SetZoom(6.0f);
    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);

    FrameBuffer::Spec spec;
    spec.Width = camera->GetViewportWidth(); spec.Height = camera->GetViewportHeight();
    spec.Attachments = {
            { FrameBuffer::Spec::AttachmentFormat::Color,			      FrameBuffer::Spec::AttachmentCategory::ReadWrite },
            { FrameBuffer::Spec::AttachmentFormat::RedInteger,          FrameBuffer::Spec::AttachmentCategory::ReadWrite },
            { FrameBuffer::Spec::AttachmentFormat::Depth24Stencil8,     FrameBuffer::Spec::AttachmentCategory::Write },
    };
    m_FrameBuffer = FrameBuffer::Create(spec);
    RenderCommand::SetClearColor(glm::vec3{ 0.1f, 0.1f, 0.1f });

    m_Scenes.push_back(CreateRef<MarioScene>());
    m_CurrentScene = m_Scenes.back();

    m_CurrentScene->SetCamera(camera.get());
    m_CurrentScene->SetFramebuffer(m_FrameBuffer.get());
    m_CurrentScene->OnInit();
}

void MarioGame::OnUpdate()
{
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);
    Render();
    ValidateViewport();

    // m_CurrentScene->OnUpdate might read from framebuffer, so we bind it.
    m_FrameBuffer->Bind();
    m_CurrentScene->OnUpdate(dt);
    m_FrameBuffer->Unbind();
}

void MarioGame::OnImguiUpdate()
{
    m_ViewportSize = ImguiMainViewport(*m_FrameBuffer);
    m_CurrentScene->OnImguiRender();
}

void MarioGame::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyPressedEvent>(BIND_FN(MarioGame::OnKeyboardPressed));
    dispatcher.Dispatch<KeyReleasedEvent>(BIND_FN(MarioGame::OnKeyboardReleased));
    m_CurrentScene->OnEvent(e);
}

bool MarioGame::OnKeyboardPressed(KeyPressedEvent& event)
{
    if (m_CurrentScene->HasAction(event.GetKeyCode()))
    {
        Action& action = m_CurrentScene->GetAction(event.GetKeyCode());
        action.SetStatus(Action::Status::Begin);
        m_CurrentScene->PerformAction(m_CurrentScene->GetAction(event.GetKeyCode()));
    }
    return false;
}

bool MarioGame::OnKeyboardReleased(KeyReleasedEvent& event)
{
    if (m_CurrentScene->HasAction(event.GetKeyCode()))
    {
        Action& action = m_CurrentScene->GetAction(event.GetKeyCode());
        action.SetStatus(Action::Status::End);
        m_CurrentScene->PerformAction(m_CurrentScene->GetAction(event.GetKeyCode()));
    }
    return false;
}

void MarioGame::Render()
{
    m_FrameBuffer->Bind();
    RenderCommand::ClearScreen();
    I32 clearInteger = -1;
    m_FrameBuffer->ClearAttachment(1, RendererAPI::DataType::Int, &clearInteger);
    Renderer2D::BeginScene(m_CameraController->GetCamera().get());

    // Render current scene.
    m_CurrentScene->OnRender();

    Renderer2D::EndScene();
    m_FrameBuffer->Unbind();
}

void MarioGame::ValidateViewport()
{
    if (FrameBuffer::Spec spec = m_FrameBuffer->GetSpec();
        m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
        (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
    {
        m_FrameBuffer->Resize((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        m_CurrentScene->SetFramebuffer(m_FrameBuffer.get());
        m_CameraController->GetCamera()->SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
        RenderCommand::SetViewport((U32)m_ViewportSize.x, (U32)m_ViewportSize.y);
    }
}

CRect MarioGame::GetCameraBounds()
{
    glm::vec2 min = m_CameraController->GetCamera()->ScreenToWorldPoint({ 0, 0 });
    glm::vec2 max = m_CameraController->GetCamera()->ScreenToWorldPoint({ m_ViewportSize.x,  m_ViewportSize.y });
    CRect bounds;
    bounds.Center = { (max.x + min.x) / 2.0f, (max.y + min.y) / 2.0f };
    bounds.HalfSize = { (max.x - min.x) / 2.0f, (max.y - min.y) / 2.0f };
    return bounds;
}
