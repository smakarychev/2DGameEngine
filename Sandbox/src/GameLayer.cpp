#include "GameLayer.h"

#include "GLFW/glfw3.h"

void GameLayer::OnAttach()
{

    m_Shader = Shader::ReadShaderFromFile("julia", "assets/shaders/test.glsl");

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

    auto ibo = IndexBuffer::Create(indices, 6, sizeof(indices));
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
    RenderCommand::ClearScreen();

    static F32 angleStep = 0.005f;
    static F32 juliaAngle = 0.0f;
    static F32 zoomStep = 0.01f;
    static F32 zoom = 1.0f;

    if (Input::GetKey(Key::A)) juliaAngle -= angleStep;
    if (Input::GetKey(Key::D)) juliaAngle += angleStep;

    if (Input::GetKey(Key::Minus)) zoom -= zoomStep;
    if (Input::GetKey(Key::Equal)) zoom += zoomStep;

    U32 renderResX = Application::Get().GetWindow().GetWidth() * 4;
    U32 renderResY = Application::Get().GetWindow().GetHeight() * 4;

    m_Shader->Bind();
    m_Shader->SetUniformFloat("u_zoom", zoom);
    m_Shader->SetUniformFloat2("u_c", glm::vec2(0.7885f * glm::cos(juliaAngle), 0.7885f * glm::sin(juliaAngle)));
    m_Shader->SetUniformFloat2("u_screenDims", glm::vec2(renderResX, renderResY));
    m_Shader->SetUniformFloat2("u_offset", glm::vec2(-(F32)renderResX / 2.0, -(F32)renderResY / 2.0));

    Renderer::BeginScene();
    Renderer::Submit(m_Shader, m_VAO, {});
    Renderer::EndScene();
}