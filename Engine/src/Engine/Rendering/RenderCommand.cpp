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
		using Unpacked = Renderer2D::DrawInfo;
		Unpacked* unpacked = (Unpacked*)(parameters);
		Renderer2D::DrawQuadCall(*unpacked);
	}
	
	void RenderCommand::DrawQuadMatBatched(void* parameters)
	{
		using Unpacked = Renderer2D::DrawInfoMat;
		Unpacked* unpacked = (Unpacked*)(parameters);
		Renderer2D::DrawQuadMatCall(*unpacked);
	}
	
	void RenderCommand::DrawPolygonBatched(void* parameters)
	{
		struct Unpacked
		{
			RegularPolygon Polygon;
			Renderer2D::DrawInfo DrawInfo;
		};
		Unpacked* unpacked = (Unpacked*)(parameters);
		Renderer2D::DrawPolygonCall(unpacked->Polygon, unpacked->DrawInfo);
	}
	
	void RenderCommand::DrawFontFixedBatched(void* parameters)
	{
		struct Unpacked
		{
			Font Font;
			F32 FontSize;
			F32 XminPx; 
			F32 XmaxPx;
			F32 YminPx; 
			std::string Text;
			glm::vec4 Color;
		};
		Unpacked* unpacked = (Unpacked*)(parameters);
		Renderer2D::DrawFontFixedCall(
			unpacked->Font,
			unpacked->FontSize,
			unpacked->XminPx,
			unpacked->XmaxPx,
			unpacked->YminPx,
			unpacked->Text,
			unpacked->Color
		);
	}
	
	void RenderCommand::DrawFontBatched(void* parameters)
	{
		struct Unpacked
		{
			Font Font;
			F32 FontSize;
			F32 Xmin;
			F32 Xmax;
			F32 Ymin;
			std::string Text;
			glm::vec4 Color;
		};
		Unpacked* unpacked = (Unpacked*)(parameters);
		Renderer2D::DrawFontFixedCall(
			unpacked->Font,
			unpacked->FontSize,
			unpacked->Xmin,
			unpacked->Xmax,
			unpacked->Ymin,
			unpacked->Text,
			unpacked->Color
		);
	}
	
	void RenderCommand::DrawLine(void* parameters)
	{
		struct Unpacked
		{
			glm::vec2 From;
			glm::vec2 To;
			glm::vec4 Color;
		};
		Unpacked* unpacked = (Unpacked*)(parameters);
		Renderer2D::DrawLineCall(unpacked->From, unpacked->To, unpacked->Color);
	}
}


