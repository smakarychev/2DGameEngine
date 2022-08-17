#pragma once

#include "Engine/Core/Types.h"

#include "Engine/Rendering/Buffer.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"	

#include "Engine/Core/Camera.h"
#include "Engine/Primitives/2D/RegularPolygon.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;
	class Renderer2D
	{
	public:
		struct BatchVertex
		{
			// vec3 for position to have the ability to render something on top.
			glm::vec3 Position		= glm::vec3(0.0f);
			F32 TextureIndex		= -1.0f;
			glm::vec2 UV			= glm::vec2(0.0f);
			glm::vec2 TextureTiling	= glm::vec2(1.0f);
			glm::vec4 Color			= glm::vec4(1.0f);

			// Maybe just static member?
			static const VertexLayout& GetLayout()
			{
				const static VertexLayout layout{ {
					{ LayoutElement::Float3,	"a_position" },
					{ LayoutElement::Float,		"a_textureIndex" },
					{ LayoutElement::Float2,	"a_uv" },
					{ LayoutElement::Float2,	"a_textureTiling" },
					{ LayoutElement::Float4,	"a_color" },
				} };
				return layout;
			}

			BatchVertex(const glm::vec3& pos, const glm::vec2& uv, const glm::vec4 color) :
				Position(pos), UV(uv), Color(color)
			{}
			BatchVertex(const glm::vec3& pos, const glm::vec2& uv, Texture& texture, F32 textureTiling = 1.0f) :
				Position(pos), UV(uv), TextureIndex(F32(texture.GetId())), TextureTiling(textureTiling)
			{}
			BatchVertex() = default;
		};

		struct BatchData
		{
			// TODO: move to config.
			U32 MaxVertices = U32(2_MiB) / sizeof(BatchVertex);
			U32 MaxIndices = U32(2_MiB) / sizeof(U32);
			U32 MaxTextures = 32;
			Texture* UsedTextures[32] = { 0 };
			U32 CurrentTextureIndex = 0;
			U32 CurrentVertices = 0;
			U32 CurrentIndices = 0;
			std::shared_ptr<VertexArray> VAO;

			U8* VerticesMemory = nullptr;
			BatchVertex* CurrentVertexPointer = nullptr;
			U8* IndicesMemory = nullptr;
			U32* CurrentIndexPointer;
		};

		struct BatchRendererData
		{
			U32 DrawCalls = 0;

			std::shared_ptr<Shader> BatchShader;
			glm::mat4 CameraViewProjection = glm::mat4(1.0f);

			struct ReferenceQuad
			{
				// Vec4 for alignment.
				std::vector<glm::vec4> Position;
				std::vector<glm::vec2> UV;
			};

			ReferenceQuad ReferenceQuad;

			BatchData QuadBatch;
			BatchData PolygonBatch;
		};

	public:
		static void Init();
		static void ShutDown();

		static void BeginScene(std::shared_ptr<Camera> camera);
		static void EndScene();

		static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, F32 rotation, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, Texture& texture, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, F32 rotation, Texture& texture, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, Texture& texture, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, F32 rotation, Texture& texture, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, Texture& texture, const std::vector<glm::vec2>& uv, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, Texture* texture, const std::vector<glm::vec2>& uv, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawQuad(
			const glm::vec3& position, const glm::vec2& scale, F32 rotation,
			Texture* texture, const std::vector<glm::vec2>& uv, 
			const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f)
		);

		static void DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color);
		static void DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, F32 rotation, const glm::vec4& color);
		static void DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, Texture& texture, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, F32 rotation, Texture& texture, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, Texture& texture, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, Texture* texture, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static void DrawPolygon(
			const RegularPolygon& polygon,
			const glm::vec3& position, const glm::vec2& scale, F32 rotation,
			Texture* texture, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f)
		);

		static void Flush(BatchData& batch);
		static void ResetBatch(BatchData& batch);
	private:
		static void InitVertexGeometryData(BatchVertex& vertex, const glm::vec3& position, const glm::vec2& scale);
		static void InitVertexGeometryData(BatchVertex& vertex, const glm::vec3& position, const glm::vec2& scale, F32 rotation);
		static void InitVertexColorData(BatchVertex& vertex, F32 textureIndex, const glm::vec2& uv, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2(1.0f));
		static F32 GetTextureIndex(BatchData& batch, Texture* texture);
	private:
		static BatchRendererData s_BatchData;
	};
}