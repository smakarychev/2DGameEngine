#pragma once

#include "Engine/Core/Camera.h"

#include "RendererAPI.h"

namespace Engine
{
	// This class exists to later have command queue for optimized rendering.
	class RenderCommand
	{
	public:
		static void Init();
		static void DrawIndexed(Ref<VertexArray> vertexArray);
		static void DrawIndexed(Ref<VertexArray> vertexArray, U32 count);
		static void DrawIndexed(Ref<VertexArray> vertexArray, RendererAPI::PrimitiveType type);
		static void DrawIndexed(Ref<VertexArray> vertexArray, U32 count, RendererAPI::PrimitiveType type);

		static void DrawQuadBatched(void* parameters);
		static void DrawQuadMatBatched(void* parameters);
		static void DrawPolygonBatched(void* parameters);
		static void DrawFontFixedBatched(void* parameters);
		static void DrawFontBatched(void* parameters);
		static void DrawLine(void* parameters);

		static void SetClearColor(const glm::vec3& color);
		static void SetDepthTestMode(RendererAPI::Mode mode);
		static void ClearScreen();
		static void SetViewport(U32 width, U32 height);
	private:
		static Ref<RendererAPI> s_API;
	};

	// To be used in render queue.
	using RenderCommandFn = void (*)(void*);
}
