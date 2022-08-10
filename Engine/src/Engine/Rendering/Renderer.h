#pragma once

#include "RendererAPI.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init();
		static void Submit(std::shared_ptr<Shader> shader, std::shared_ptr<VertexArray> vertexArray, const glm::mat3& transform);
		static void BeginScene();
		static void EndScene();

		static RendererAPI::APIType GetAPI();
	};
}