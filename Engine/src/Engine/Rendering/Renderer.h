#pragma once

#include "RendererAPI.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init();
		static void ShutDown();
		static void Submit(Ref<Shader> shader, Ref<VertexArray> vertexArray, const glm::mat4& transform = glm::mat4(1.0f));
		static void BeginScene();
		static void EndScene();
		static void OnWindowResize(U32 width, U32 height);

		static RendererAPI::APIType GetAPI();
	};
}