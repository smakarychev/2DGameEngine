#include "enginepch.h"

#include "RenderCommand.h"
#include "Renderer2D.h"

namespace Engine
{

	Ref<RendererAPI> RenderCommand::s_API = RendererAPI::Create();

	void RenderCommand::Init()
	{
		s_API->Init();
	}

	void RenderCommand::DrawIndexed(Ref<VertexArray> vertexArray)
	{
		s_API->DrawIndexed(vertexArray);
	}

	void RenderCommand::DrawIndexed(Ref<VertexArray> vertexArray, U32 count)
	{
		s_API->DrawIndexed(vertexArray, count);
	}

	void RenderCommand::DrawIndexed(Ref<VertexArray> vertexArray, RendererAPI::PrimitiveType type)
	{
		s_API->DrawIndexed(vertexArray, type);
	}

	void RenderCommand::DrawIndexed(Ref<VertexArray> vertexArray, U32 count, RendererAPI::PrimitiveType type)
	{
		s_API->DrawIndexed(vertexArray, count, type);
	}

	void RenderCommand::EnableCull(bool enable)
	{
		s_API->EnableCull(enable);
	}

	void RenderCommand::SetCullFrontFace(RendererAPI::CullFrontFace face)
	{
		s_API->SetCullFrontFace(face);
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

	void RenderCommand::SetViewport(U32 width, U32 height)
	{
		s_API->SetViewport(width, height);
	}

	void RenderCommand::DrawQuadBatched(void* parameters)
	{
		
	}
	
	void RenderCommand::DrawQuadMatBatched(void* parameters)
	{
		
	}
	
	void RenderCommand::DrawPolygonBatched(void* parameters)
	{
		
	}
	
	void RenderCommand::DrawFontFixedBatched(void* parameters)
	{
		
	}
	
	void RenderCommand::DrawFontBatched(void* parameters)
	{
	
	}
	
	void RenderCommand::DrawLine(void* parameters)
	{
		
	}
}


