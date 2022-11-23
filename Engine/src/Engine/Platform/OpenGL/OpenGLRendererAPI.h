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
		void DrawIndexed(Ref<VertexArray> vertexArray) override;
		void DrawIndexed(Ref<VertexArray> vertexArray, U32 count) override;
		void DrawIndexed(Ref<VertexArray> vertexArray, PrimitiveType type) override;
		void DrawIndexed(Ref<VertexArray> vertexArray, U32 count, PrimitiveType type) override;	

		void EnableCull(bool enable) override;
		void SetCullFrontFace(CullFrontFace face) override;
		void SetDepthTestMode(Mode mode) override;
		void SetViewport(U32 width, U32 height) override;

		static U32 GetNativeDataType(RendererAPI::DataType type);
	private:
		glm::vec3 m_ClearColor = {0.0f, 0.0f, 1.0f};
	};
}
