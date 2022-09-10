#include "enginepch.h"
#include "Renderer.h"

#include "RenderCommand.h"
#include "Renderer2D.h"

namespace Engine
{
	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Submit(Ref<Shader> shader, Ref<VertexArray> vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		shader->SetUniformMat4("u_modelViewProjection", transform);
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::BeginScene()
	{
		
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::OnWindowResize(U32 width, U32 height)
	{
		RenderCommand::SetViewport(width, height);
	}

	RendererAPI::APIType Engine::Renderer::GetAPI()
	{
		return RendererAPI::Get();
	}

	void Renderer::ShutDown()
	{
		Renderer2D::ShutDown();
	}
}


