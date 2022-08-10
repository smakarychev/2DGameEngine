#include "enginepch.h"
#include "Renderer.h"

#include "RenderCommand.h"

namespace Engine
{
	void Engine::Renderer::Init()
	{
		RenderCommand::Init();
	}

	void Engine::Renderer::Submit(std::shared_ptr<Shader> shader, std::shared_ptr<VertexArray> vertexArray, const glm::mat3& transform)
	{
		RenderCommand::DrawIndexed(shader, vertexArray);
	}

	void Engine::Renderer::BeginScene()
	{
		
	}

	void Engine::Renderer::EndScene()
	{
	}

	RendererAPI::APIType Engine::Renderer::GetAPI()
	{
		return RendererAPI::Get();
	}
}


