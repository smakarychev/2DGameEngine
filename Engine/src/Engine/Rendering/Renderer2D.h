#pragma once

#include "Engine/Core/Types.h"

#include "Engine/Rendering/Buffer.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"	
#include "Engine/Rendering/Font.h"	

#include "Engine/Rendering/RendererAPI.h"

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
			glm::vec2 TextureTiling	= glm::vec2{1.0f};
			glm::vec4 Color			= glm::vec4{ 1.0f };

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
				Position(pos), TextureIndex(F32(texture.GetId())), UV(uv), TextureTiling(textureTiling)
			{}
			BatchVertex() = default;
		};

		struct BatchVertexLine
		{
			glm::vec3 Position;
			glm::vec4 Color;

			static const VertexLayout& GetLayout()
			{
				const static VertexLayout layout{ {
					{ LayoutElement::Float3,	"a_position" },
					{ LayoutElement::Float4,	"a_color" },
				} };
				return layout;
			}

			BatchVertexLine(const glm::vec3& pos, const glm::vec4 color) :
				Position(pos), Color(color)
			{}
			BatchVertexLine() = default;
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
			Ref<VertexArray> VAO;

			U8* VerticesMemory = nullptr;
			BatchVertex* CurrentVertexPointer = nullptr;
			U8* IndicesMemory = nullptr;
			U32* CurrentIndexPointer = nullptr;
		};

		struct BatchDataLines
		{
			// TODO: move to config.
			U32 MaxVertices = U32(2_MiB) / sizeof(BatchVertexLine);
			U32 MaxIndices = U32(2_MiB) / sizeof(U32);
			U32 CurrentVertices = 0;
			U32 CurrentIndices = 0;
			Ref<VertexArray> VAO;

			U8* VerticesMemory = nullptr;
			BatchVertexLine* CurrentVertexPointer = nullptr;
			U8* IndicesMemory = nullptr;
			U32* CurrentIndexPointer = nullptr;
		};

		struct BatchRendererData
		{
			U32 DrawCalls = 0;

			Ref<Shader> BatchShader;
			Ref<Shader> TextShader;
			Ref<Shader> LineShader;
			glm::mat4 CameraViewProjection = glm::mat4(1.0f);
			Camera* Camera = nullptr;

			struct ReferenceQuad
			{
				// Vec4 for alignment.
				std::vector<glm::vec4> Position;
				std::vector<glm::vec2> UV;
			};

			ReferenceQuad ReferenceQuad;

			BatchData QuadBatch;
			BatchData PolygonBatch;
			BatchData TextBatch;
			BatchDataLines LineBatch;
		};

		struct DrawInfo
		{
			struct Rotation
			{
				glm::vec2 RotationVec;
				Rotation(const glm::vec2& rotation) : RotationVec(rotation) {}
				Rotation(F32 angleRad) : RotationVec(glm::cos(angleRad), glm::sin(angleRad)) {}
			};
			const glm::vec3& Position			= glm::vec3{ 0.0f };
			const glm::vec2& Scale				= glm::vec2{ 1.0f };
			const Rotation&  Rotation			= glm::vec2{ 1.0f, 0.0f };
			const glm::vec4& Color				= glm::vec4{ 1.0f };
			Texture*  Texture					= nullptr;
			const std::vector<glm::vec2>& UV	= { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
			const glm::vec2& TextureTiling		= glm::vec2{ 1.0f };
			RendererAPI::PrimitiveType Type		= RendererAPI::PrimitiveType::Triangle;
		};

	public:
		static void Init();
		static void ShutDown();

		static void BeginScene(Ref<Camera> camera);
		static void EndScene();

		static void DrawQuad(const DrawInfo& drawInfo);

		static void DrawPolygon(const RegularPolygon& polygon, const DrawInfo& drawInfo);

		// Winapi uses DrawText as a macro >:(
		// Fixed font retains its relative position and size as camera moves/zoomes.
		static void DrawFontFixed(Font& font, F32 fontSize, const std::string& text, const glm::vec4& color = glm::vec4{ 1.0f });
		static void DrawFontFixed(Font& font, F32 fontSize, F32 xminPx, F32 xmaxPx, F32 yminPx, const std::string& text, const glm::vec4& color = glm::vec4{ 1.0f });

		// `fontSize` as if camera is exactly one unit away.
		static void DrawFont(Font& font, F32 fontSize, F32 xmin, F32 xmax, F32 ymin, const std::string& text, const glm::vec4& color = glm::vec4{ 1.0f });

		static void DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color = glm::vec4{ 1.0f });
		static void DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color = glm::vec4{ 1.0f });

		static void Flush(BatchData& batch, Shader& shader = *s_BatchData.BatchShader);
		static void Flush(BatchDataLines& batch, Shader& shader = *s_BatchData.LineShader);
		static void ResetBatch(BatchData& batch);
		static void ResetBatch(BatchDataLines& batch);
	private:
		static void InitVertexGeometryData(BatchVertex& vertex, const glm::vec3& position, const glm::vec2& scale);
		static void InitVertexGeometryData(BatchVertex& vertex, const glm::vec3& position, const glm::vec2& scale, F32 rotation);
		static void InitVertexGeometryData(BatchVertex& vertex, const glm::vec3& position, const glm::vec2& scale, const glm::vec2& rotation);
		static void InitVertexColorData(BatchVertex& vertex, F32 textureIndex, const glm::vec2& uv, const glm::vec4& tint, const glm::vec2& textureTiling = glm::vec2{ 1.0f });
		static F32 GetTextureIndex(BatchData& batch, Texture* texture);
		static void DrawOutline(const DrawInfo& drawInfo);
	private:
		static BatchRendererData s_BatchData;
	};
}