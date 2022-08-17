#include "GameLayer.h"

#include "GLFW/glfw3.h"

void GameLayer::OnAttach()
{

    m_Shader = Shader::ReadShaderFromFile("assets/shaders/julia.glsl");
    m_Texture = Texture::LoadTextureFromFile("assets/textures/checker.png");
    m_Brick = Texture::LoadTextureFromFile("assets/textures/brick.jpg");
    m_Tree = Texture::LoadTextureFromFile("assets/textures/tree.png");
    m_Tileset = Texture::LoadTextureFromFile("assets/textures/tilesheet.png");
    m_Tile = m_Tileset->GetSubTexture({ 64.0f, 64.0f }, { 18, 12 }, { 2, 2 });
    auto camera = Camera::Create(glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 16.0f / 9.0f);

    m_CameraController = CameraController::Create(CameraController::ControllerType::Editor2D, camera);

    F32 vertices[] = 
    {
        -0.8f,  0.8f, 0.0f, 0.0f, 1.0f,
        -0.8f, -0.8f, 0.0f, 0.0f, 0.0f,
         0.8f, -0.8f, 0.0f, 1.0f, 0.0f,
         0.8f,  0.8f, 0.0f, 1.0f, 1.0f,
    };

    U32 indices[] =
    {
        0, 1, 2,
        2, 0, 3,
    };

    auto ibo = IndexBuffer::Create(indices, 6);
    auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
    VertexLayout bufferLayout{ {
        { LayoutElement::Float3, "a_position"},
        { LayoutElement::Float2, "a_texcoord"}
    } };
    vbo->SetVertexLayout(bufferLayout);

    m_VAO = VertexArray::Create();
    m_VAO->SetIndexBuffer(ibo);
    m_VAO->AddVertexBuffer(vbo);

    RenderCommand::SetClearColor(glm::vec3(0.1f, 0.1f, 0.1f));
}

void GameLayer::OnUpdate()
{
    F32 dt = 1.0f / 60.0f;
    m_CameraController->OnUpdate(dt);
    RenderCommand::ClearScreen();

    if (Input::GetKey(Key::P)) m_CameraController->GetCamera()->SetProjection(Camera::ProjectionType::Perspective);
    if (Input::GetKey(Key::O)) m_CameraController->GetCamera()->SetProjection(Camera::ProjectionType::Orthographic);

    U32 renderResX = Application::Get().GetWindow().GetWidth() * 4;
    U32 renderResY = Application::Get().GetWindow().GetHeight() * 4;

    Renderer2D::BeginScene(m_CameraController->GetCamera());
    
    for (U32 i = 3; i < 10; i++)
    {
        RegularPolygon pol{ i, false };
        pol.GenerateUVs(m_Tileset->GetSubTextureUV({ 64.0f, 64.0f }, { 22, 15 }, { 1, 1 }));

        Renderer2D::DrawPolygon(pol, glm::vec3(F32(i - 3), 0.0f, 0.1f), glm::vec2(0.5f, 0.5f), glm::sin(glfwGetTime()), *m_Tileset, glm::vec2(1.0f));
    }
    F32 tileSize = 0.5f;
    for (U32 i = 0; i < 100; i++)
    {
        for (U32 j = 0; j < 100; j++)
        {
            if ((i + j) % 2 == 0)
            {
                Renderer2D::DrawQuad(glm::vec3(i * tileSize,j * tileSize, 0.0), glm::vec2(tileSize * 0.9, tileSize * 0.9), *m_Brick);
            }
            else
            {
                Renderer2D::DrawQuad(
                    glm::vec3(i * tileSize,j * tileSize, 0.0),
                    glm::vec2(tileSize * 0.9, tileSize * 0.9),
                    glm::radians(45.0f),
                    m_Tileset.get(),
                    m_Tileset->GetSubTextureUV({ 64.0f, 64.0f }, { 20, 15 }, { 1, 1 }),
                    glm::vec4(1.0f)
                );
            }
            
        }
    }

    Renderer2D::EndScene();
}

void GameLayer::OnEvent(Event& event)
{
    m_CameraController->OnEvent(event);
}
