#pragma once

#include "RendererAPI.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init();
		static void ShutDown();
		static void Submit(std::shared_ptr<Shader> shader, std::shared_ptr<VertexArray> vertexArray, const glm::mat4& transform = glm::mat4(1.0f));
		static void BeginScene();
		static void EndScene();
		static void OnWindowResize(U32 width, U32 height);

		static RendererAPI::APIType GetAPI();
	};
}