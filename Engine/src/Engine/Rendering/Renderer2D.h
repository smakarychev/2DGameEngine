#pragma once

#include "Engine/Core/Types.h"

#include "Engine/Rendering/Buffer.h"
#include "Engine/Rendering/Shader.h"

#include "Engine/Core/Camera.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;
	class Renderer2D
	{
	public:
		// TODO: reasonable name? (This is from Game engine architecture book.
		struct Vertex1P1UV1C
		{
			// vec3 for position to have the ability to render something on top.
			glm::vec3 Position;
			glm::vec2 UV;
			glm::vec4 Color;

			// Maybe just static member?
			static const VertexLayout& GetLayout()
			{
				const static VertexLayout layout{ {
					{ LayoutElement::Float3, "a_position" },
					{ LayoutElement::Float2, "a_uv" },
					{ LayoutElement::Float4, "a_color" },
				} };
				return layout;
			}

			Vertex1P1UV1C(const glm::vec3& pos, const glm::vec2& uv, const glm::vec4 color) : 
				Position(pos), UV(uv), Color(color)
			{}
			Vertex1P1UV1C() = default;
		};

		struct BatchRendererData
		{
			// TODO: move to config.
			U32 MaxVertices = 2_MiB / sizeof(Vertex1P1UV1C);
			U32 CurrentVertices = 0;
			U32 CurrentQuads = 0;

			U32 DrawCalls = 0;

			std::shared_ptr<Shader> BatchShader;
			std::shared_ptr<VertexArray> BatchVAO;

			struct ReferenceQuad
			{
				// Vec4 for alignment.
				glm::vec4 Position[4];
				glm::vec4 Color[4];
				glm::vec2 UV[4];
			};

			U8* QuadsVerticesMemory = nullptr;
			Vertex1P1UV1C* CurrentQuadVertexPointer = nullptr;

			ReferenceQuad ReferenceQuad;
			glm::mat4 CameraViewProjection = glm::mat4(1.0f);
		};

	public:
		static void Init();
		static void ShutDown();

		static void BeginScene(std::shared_ptr<Camera> camera);
		static void EndScene();

		static void DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& color);
		static void Flush();
		static void ResetBatch();

	private:
		static BatchRendererData s_BatchData;
	};
}