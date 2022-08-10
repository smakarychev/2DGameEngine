#include "enginepch.h"
#include "RenderCommand.h"


namespace Engine
{

	std::shared_ptr<RendererAPI> RenderCommand::s_API = RendererAPI::Create();

	void RenderCommand::Init()
	{
		s_API->Init();
	}

	void RenderCommand::DrawIndexed(std::shared_ptr<Shader> shader, std::shared_ptr<VertexArray> vertexArray)
	{
		s_API->DrawIndexed(shader, vertexArray);
	}

	void RenderCommand::SetClearColor(const glm::vec3& color)
	{
		s_API->SetClearColor(color);
	}

	void RenderCommand::ClearScreen()
	{
		s_API->ClearScreen();
	}
}


