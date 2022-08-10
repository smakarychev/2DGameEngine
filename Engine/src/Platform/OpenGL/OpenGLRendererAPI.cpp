#include "enginepch.h"
#include "OpenGLRendererAPI.h"

#include "glad/glad.h"

void Engine::OpenGLRendererAPI::Init()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Engine::OpenGLRendererAPI::ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::OpenGLRendererAPI::SetClearColor(const glm::vec3& color)
{
	m_ClearColor = color;
	glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, 1.0f);
}

void Engine::OpenGLRendererAPI::DrawIndexed(std::shared_ptr<Shader> shader, std::shared_ptr<VertexArray> vertexArray)
{
	shader->Bind();
	vertexArray->Bind();
	glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
}
