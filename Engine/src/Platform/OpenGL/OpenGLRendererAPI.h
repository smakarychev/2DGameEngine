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
		void DrawIndexed(std::shared_ptr<Shader> shader, std::shared_ptr<VertexArray> vertexArray) override;
	private:
		void ErrorCallback(U32 source, U32 type, U32 id, U32 severity, I32 length, U8 const* message, void const* user_param);
	private:
		glm::vec3 m_ClearColor;
	};
}
