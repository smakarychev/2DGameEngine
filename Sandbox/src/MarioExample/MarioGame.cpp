#include "MarioGame.h"

#include "MarioScene.h"

void MarioGame::OnAttach()
{
    RenderCommand::SetClearColor(glm::vec3{ 0.1f, 0.1f, 0.1f });

    m_Scenes.push_back(CreateRef<MarioScene>());
    m_CurrentScene = m_Scenes.back();
    m_CurrentScene->OnInit();
}

void MarioGame::OnUpdate()
{
    F32 dt = 1.0f / 60.0f;
    // m_CurrentScene->OnUpdate might read from framebuffer, so we bind it.
    m_CurrentScene->OnUpdate(dt);

    Render();
}

void MarioGame::OnImguiUpdate()
{
    m_CurrentScene->OnImguiUpdate();
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
    // Render current scene.
    m_CurrentScene->OnRender();
}
