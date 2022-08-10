#pragma once

#include "RendererAPI.h"

namespace Engine
{
	// This class exists to later have command queue for optimized rendering.
	class RenderCommand
	{
	public:
		static void Init();
		static void DrawIndexed(std::shared_ptr<Shader> shader, std::shared_ptr<VertexArray> vertexArray);
		static void SetClearColor(const glm::vec3& color);
		static void ClearScreen();
	private:
		static std::shared_ptr<RendererAPI> s_API;
	};
}
