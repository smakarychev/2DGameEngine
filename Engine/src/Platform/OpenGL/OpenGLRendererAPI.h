#pragma once

#include "Engine/Rendering/RendererAPI.h"

namespace Engine
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		void ClearScreen() override;
		void SetClearColor(const glm::vec3& color) override;
		void DrawIndexed(std::shared_ptr<VertexArray> vertexArray) override;
		void DrawIndexed(std::shared_ptr<VertexArray> vertexArray, U32 count) override;

		void SetDepthTestMode(Mode mode) override;
		void SetViewport(U32 width, U32 height) override;
	private:
		glm::vec3 m_ClearColor;
	};
}
