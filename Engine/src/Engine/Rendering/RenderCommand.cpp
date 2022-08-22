#include "enginepch.h"
#include "RenderCommand.h"


namespace Engine
{

	std::shared_ptr<RendererAPI> RenderCommand::s_API = RendererAPI::Create();

	void RenderCommand::Init()
	{
		s_API->Init();
	}

	void RenderCommand::DrawIndexed(std::shared_ptr<VertexArray> vertexArray)
	{
		s_API->DrawIndexed(vertexArray);
	}

	void RenderCommand::DrawIndexed(std::shared_ptr<VertexArray> vertexArray, U32 count)
	{
		s_API->DrawIndexed(vertexArray, count);
	}

	void RenderCommand::SetClearColor(const glm::vec3& color)
	{
		s_API->SetClearColor(color);
	}

	void RenderCommand::SetDepthTestMode(RendererAPI::Mode mode)
	{
		s_API->SetDepthTestMode(mode);
	}

	void RenderCommand::ClearScreen()
	{
		s_API->ClearScreen();
	}
}


