#include "GameLayer.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

void GameLayer::OnAttach()
{
    // TODO: Move to renderer.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    auto ibo = IndexBuffer::Create(indices, sizeof(indices));
    auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
    VertexLayout bufferLayout{ {
        { LayoutElement::Float3, "a_position"},
        { LayoutElement::Float2, "a_texcoord"}
    } };
    vbo->SetVertexLayout(bufferLayout);

    m_VAO = VertexArray::Create();
    m_VAO->SetIndexBuffer(ibo);
    m_VAO->AddVertexBuffer(vbo);
}

void GameLayer::OnUpdate()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

    static F32 angleStep = 0.005f;
    static F32 juliaAngle = 0.0f;
    static F32 zoomStep = 0.01f;
    static F32 zoom = 1.0f;

    if (Input::GetKey(Key::A)) juliaAngle -= angleStep;
    if (Input::GetKey(Key::D)) juliaAngle += angleStep;

    if (Input::GetKey(Key::Minus)) zoom -= zoomStep;
    if (Input::GetKey(Key::Equal)) zoom += zoomStep;

    m_Shader->Bind();
    m_Shader->SetUniformFloat("u_zoom", zoom);
    m_Shader->SetUniformFloat2("u_c", glm::vec2(0.7885f * glm::cos(juliaAngle), 0.7885f * glm::sin(juliaAngle)));
    m_Shader->SetUniformFloat2("u_screenDims", glm::vec2(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight()));
    m_Shader->SetUniformFloat2("u_offset", glm::vec2(-(F32)Application::Get().GetWindow().GetWidth() / 2.0, -(F32)Application::Get().GetWindow().GetHeight() / 2.0));

    m_VAO->Bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}