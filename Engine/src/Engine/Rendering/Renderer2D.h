#pragma once

#include "Engine/Core/Types.h"

#include "Buffer.h"
#include "RendererAPI.h"
#include "RenderQueue.h"
#include "Shader.h"
#include "SortingLayer.h"
#include "Texture.h"
#include "Font.h"

#include "Engine/ECS/Components.h"
#include "Engine/Core/Camera.h"
#include "Engine/Primitives/2D/RegularPolygon.h"

#include <glm/glm.hpp>

namespace Engine
{
    using namespace Types;

    class Renderer2D
    {
        friend class RenderCommand;
    public:
        struct BatchVertex
        {
            // vec3 for position to have the ability to render something on top.
            glm::vec3 Position = glm::vec3{0.0f};
            F32 TextureIndex = -1.0f;
            glm::vec2 UV = glm::vec2{0.0f};
            glm::vec2 TextureTiling = glm::vec2{1.0f};
            glm::vec4 Color = glm::vec4{1.0f};

            // Maybe just static member?
            static const VertexLayout& GetLayout()
            {
                const static VertexLayout layout{
                    {
                        {LayoutElement::Float3, "a_position"},
                        {LayoutElement::Float, "a_textureIndex"},
                        {LayoutElement::Float2, "a_uv"},
                        {LayoutElement::Float2, "a_textureTiling"},
                        {LayoutElement::Float4, "a_color"},
                    }
                };
                return layout;
            }

            BatchVertex(const glm::vec3& pos, const glm::vec2& uv, const glm::vec4 color) :
                Position(pos), UV(uv), Color(color)
            {
            }

            BatchVertex(const glm::vec3& pos, const glm::vec2& uv, const Texture& texture,
                        const glm::vec2& textureTiling = glm::vec2{1.0f}) :
                Position(pos), TextureIndex(F32(texture.GetId())), UV(uv), TextureTiling(textureTiling)
            {
            }

            BatchVertex() = default;
        };

        struct BatchVertexEditor
        {
            // vec3 for position to have the ability to render something on top.
            I32 EntityId = -1;
            glm::vec3 Position = glm::vec3{0.0f};
            F32 TextureIndex = -1.0f;
            glm::vec2 UV = glm::vec2{0.0f};
            glm::vec2 TextureTiling = glm::vec2{1.0f};
            glm::vec4 Color = glm::vec4{1.0f};

            // Maybe just static member?
            static const VertexLayout& GetLayout()
            {
                const static VertexLayout layout{
                    {
                        {LayoutElement::Float3, "a_position"},
                        {LayoutElement::Float, "a_textureIndex"},
                        {LayoutElement::Float2, "a_uv"},
                        {LayoutElement::Float2, "a_textureTiling"},
                        {LayoutElement::Float4, "a_color"},
                        {LayoutElement::Int, "a_enityId"},
                    }
                };
                return layout;
            }

            BatchVertexEditor(I32 entityId, const glm::vec3& pos, const glm::vec2& uv, const glm::vec4 color) :
                EntityId(entityId), Position(pos), UV(uv), Color(color)
            {
            }

            BatchVertexEditor(I32 entityId, const glm::vec3& pos, const glm::vec2& uv, const Texture& texture,
                              const glm::vec2& textureTiling = glm::vec2{1.0f}) :
                EntityId(entityId), Position(pos), TextureIndex(F32(texture.GetId())), UV(uv),
                TextureTiling(textureTiling)
            {
            }

            BatchVertexEditor() = default;
        };

        struct BatchVertexLine
        {
            glm::vec3 Position = glm::vec3{0.0f};
            glm::vec4 Color = glm::vec4{1.0f};

            static const VertexLayout& GetLayout()
            {
                const static VertexLayout layout{
                    {
                        {LayoutElement::Float3, "a_position"},
                        {LayoutElement::Float4, "a_color"},
                    }
                };
                return layout;
            }

            BatchVertexLine(const glm::vec3& pos, const glm::vec4 color) :
                Position(pos), Color(color)
            {
            }

            BatchVertexLine() = default;
        };

        struct BatchVertexLineEditor
        {
            I32 EntityId = -1;
            glm::vec3 Position = glm::vec3{0.0f};
            glm::vec4 Color = glm::vec4{1.0f};

            static const VertexLayout& GetLayout()
            {
                const static VertexLayout layout{
                    {
                        {LayoutElement::Float3, "a_position"},
                        {LayoutElement::Float4, "a_color"},
                        {LayoutElement::Int, "a_entityId"},
                    }
                };
                return layout;
            }

            BatchVertexLineEditor(I32 entityId, const glm::vec3& pos, const glm::vec4 color) :
                EntityId(entityId), Position(pos), Color(color)
            {
            }

            BatchVertexLineEditor() = default;
        };

        template <typename VT>
        struct BatchDataT
        {
            // TODO: move to config.
            U32 MaxVertices = U32(2_MiB) / sizeof(VT);
            U32 MaxIndices = U32(2_MiB) / sizeof(U32);
            U32 MaxTextures = 32;
            Texture* UsedTextures[32] = {nullptr};
            U32 CurrentTextureIndex = 0;
            U32 CurrentVertices = 0;
            U32 CurrentIndices = 0;
            Ref<VertexArray> VAO;

            U8* VerticesMemory = nullptr;
            VT* CurrentVertexPointer = nullptr;
            U8* IndicesMemory = nullptr;
            U32* CurrentIndexPointer = nullptr;
        };

        template <typename VT>
        struct BatchDataLinesT
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

        using BatchData = BatchDataT<BatchVertex>;
        using BatchDataEditor = BatchDataT<BatchVertexEditor>;
        using BatchDataLines = BatchDataLinesT<BatchVertexLine>;
        using BatchDataLinesEditor = BatchDataLinesT<BatchVertexLineEditor>;

        struct BatchRendererData
        {
            U32 DrawCalls = 0;

            Ref<Shader> BatchShader;
            Ref<Shader> TextShader;
            Ref<Shader> LineShader;

            Ref<Shader> BatchShaderEditor;
            Ref<Shader> TextShaderEditor;
            Ref<Shader> LineShaderEditor;

            glm::mat4 CameraViewProjection = glm::mat4(1.0f);
            Camera* Camera = nullptr;

            struct ReferenceQuad
            {
                // Vec4 for alignment.
                std::array<glm::vec4, 4> Position;
                std::array<glm::vec2, 4> UV;
            };

            ReferenceQuad ReferenceQuad{};

            BatchData QuadBatch;
            BatchData PolygonBatch;
            BatchData TextBatch;
            BatchDataLines LineBatch;

            BatchDataEditor QuadBatchEditor;
            BatchDataEditor PolygonBatchEditor;
            BatchDataEditor TextBatchEditor;
            BatchDataLinesEditor LineBatchEditor;

            SortingLayer* SortingLayer = nullptr;
            RenderQueue RenderQueue;
            ~BatchRendererData() = default;
        };

    public:
        Renderer2D()
        {
        }

        ~Renderer2D()
        {
        }

        static void Init();
        static void ShutDown();

        static void BeginScene(Ref<Camera> camera, SortingLayer* sortingLayer = &DefaultSortingLayer);
        static void EndScene();

        static void DrawQuad(const Component::Transform2D& transform, const Component::SpriteRenderer& spriteRenderer,
                             RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawQuad(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                             RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygon(const Component::Transform2D& transform,
                                const Component::PolygonRenderer& polygonRenderer);
        static void DrawPolygon(const glm::mat3& transform, const Component::PolygonRenderer& polygonRenderer);
        // WinApi uses DrawText as a macro >:(
        // `fontSize` as if camera is exactly one unit away.
        static void DrawFont(const Component::FontRenderer& fontRenderer, const std::string& text);
        // Fixed font retains its relative position and size as camera moves/zooms.
        static void DrawFontFixed(const Component::FontRenderer& fontRenderer, const std::string& text);
        static void DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color = glm::vec4{1.0f});

        static void DrawQuadEditor(I32 entityId, const Component::Transform2D& transform,
                                   const Component::SpriteRenderer& spriteRenderer,
                                   RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawQuadEditor(I32 entityId, const glm::mat3& transform,
                                   const Component::SpriteRenderer& spriteRenderer,
                                   RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonEditor(I32 entityId, const Component::Transform2D& transform,
                                      const Component::PolygonRenderer& polygonRenderer);
        static void DrawPolygonEditor(I32 entityId, const glm::mat3& transform,
                                      const Component::PolygonRenderer& polygonRenderer);
        static void DrawFontEditor(I32 entityId, const Component::FontRenderer& fontRenderer, const std::string& text);
        static void DrawFontFixedEditor(I32 entityId, const Component::FontRenderer& fontRenderer,
                                        const std::string& text);
        static void DrawLineEditor(I32 entityId, const glm::vec2& from, const glm::vec2& to, const glm::vec4& color);

        template <typename VT>
        static void Flush(BatchDataT<VT>& batch, Shader& shader = *s_BatchData.BatchShader);
        template <typename VT>
        static void Flush(BatchDataLinesT<VT>& batch, Shader& shader = *s_BatchData.LineShader);
        template <typename VT>
        static void ResetBatch(BatchDataT<VT>& batch);
        template <typename VT>
        static void ResetBatch(BatchDataLinesT<VT>& batch);
    private:
        template <typename VT>
        static void CheckForFlush(BatchDataT<VT>& batch, U32 verticesCount, U32 indicesCount);
        template <typename VT>
        static void CheckForFlush(BatchDataLinesT<VT>& batch, U32 verticesCount, U32 indicesCount);

        // These functions are to be executed by a command queue (currently unimplemented).

        static void DrawQuadCall(const Component::Transform2D& transform,
                                 const Component::SpriteRenderer& spriteRenderer, F32 depth = 0.0f,
                                 RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawQuadCall(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                                 F32 depth = 0.0f,
                                 RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonCall(const Component::Transform2D& transform,
                                    const Component::PolygonRenderer& polygonRenderer, F32 depth = 0.0f);
        static void DrawPolygonCall(const glm::mat3& transform2D, const Component::PolygonRenderer& polygonRenderer,
                                    F32 depth = 0.0f);
        static void DrawFontCall(const Component::FontRenderer& fontRenderer, const std::string& text,
                                 F32 depth = 0.0f);
        static void DrawFontFixedCall(const Component::FontRenderer& fontRenderer, const std::string& text);
        static void DrawLineCall(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color, F32 depth = 0.0f);

        static void DrawQuadEditorCall(I32 entityId, const Component::Transform2D& transform,
                                       const Component::SpriteRenderer& spriteRenderer, F32 depth = 0.0f,
                                       RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawQuadEditorCall(I32 entityId, const glm::mat3& transform,
                                       const Component::SpriteRenderer& spriteRenderer, F32 depth = 0.0f,
                                       RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonEditorCall(I32 entityId, const Component::Transform2D& transform,
                                          const Component::PolygonRenderer& polygonRenderer, F32 depth = 0.0f);
        static void DrawPolygonEditorCall(I32 entityId, const glm::mat3& transform,
                                          const Component::PolygonRenderer& polygonRenderer, F32 depth = 0.0f);
        static void DrawFontEditorCall(I32 entityId, const Component::FontRenderer& fontRenderer,
                                       const std::string& text, F32 depth = 0.0f);
        static void DrawFontFixedEditorCall(I32 entityId, const Component::FontRenderer& fontRenderer,
                                            const std::string& text);
        static void DrawLineEditorCall(I32 entityId, const glm::vec2& from, const glm::vec2& to, const glm::vec4& color,
                                       F32 depth = 0.0f);

        static void DrawOutlineCall(const Component::Transform2D& transform,
                                    const Component::SpriteRenderer& spriteRenderer, F32 depth = 0.0f);
        static void DrawOutlineCall(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                                    F32 depth = 0.0f);

        static F32 GetTextureIndex(BatchData& batch, Texture* texture);

        template <typename VT>
        static void PushVertex(BatchDataT<VT>& batch, const glm::vec2& referenceVertex,
                               const Component::Transform2D& transform, F32 depth, F32 textureIndex,
                               const glm::vec2& uv, const glm::vec4& tint,
                               const glm::vec2& tiling);

        template <typename VT>
        static void PushVertex(BatchDataT<VT>& batch, const glm::vec2& referenceVertex,
                               const glm::mat3& transform, F32 depth, F32 textureIndex, const glm::vec2& uv,
                               const glm::vec4& tint,
                               const glm::vec2& tiling);

        template <typename VT>
        static void InitVertexGeometryData(VT& vertex,
                                           const glm::vec3& position,
                                           const glm::vec2& scale,
                                           const glm::vec2& rotation);

        template <typename VT>
        static void InitVertexColorData(VT& vertex,
                                        F32 textureIndex,
                                        const glm::vec2& uv,
                                        const glm::vec4& tint,
                                        const glm::vec2& textureTiling = glm::vec2{1.0f});

    private:
        static BatchRendererData s_BatchData;
    };

    template <typename VT>
    void Renderer2D::Flush(BatchDataT<VT>& batch, Shader& shader)
    {
        shader.Bind();
        for (U32 i = 0; i < batch.CurrentTextureIndex; i++) batch.UsedTextures[i]->Bind(i);
        ENGINE_CORE_ASSERT(batch.VAO->GetVertexBuffers().size() == 1, "Batch must have only one vbo.")

        batch.VAO->GetVertexBuffers()[0]->SetData(batch.VerticesMemory, sizeof(VT) * batch.CurrentVertices);
        if ((void*)&batch == (void*)&s_BatchData.PolygonBatch || (void*)&batch == (void*)&s_BatchData.
            PolygonBatchEditor)
            batch.VAO->GetIndexBuffer()->SetData(reinterpret_cast<U32*>(batch.IndicesMemory), batch.CurrentIndices);
        if ((void*)&batch == (void*)&s_BatchData.TextBatch || (void*)&batch == (void*)&s_BatchData.TextBatchEditor)
            RenderCommand::SetDepthTestMode(RendererAPI::Mode::Read);

        shader.SetUniformMat4("u_modelViewProjection", s_BatchData.CameraViewProjection);
        s_BatchData.DrawCalls++;
        RenderCommand::DrawIndexed(batch.VAO, batch.CurrentIndices);
        if ((void*)&batch == (void*)&s_BatchData.TextBatch || (void*)&batch == (void*)&s_BatchData.TextBatchEditor)
            RenderCommand::SetDepthTestMode(RendererAPI::Mode::ReadWrite);
    }

    template <typename VT>
    void Renderer2D::Flush(BatchDataLinesT<VT>& batch, Shader& shader)
    {
        shader.Bind();
        ENGINE_CORE_ASSERT(batch.VAO->GetVertexBuffers().size() == 1, "Batch must have only one vbo.")
        batch.VAO->GetVertexBuffers()[0]->SetData(batch.VerticesMemory, sizeof(VT) * batch.CurrentVertices);

        shader.SetUniformMat4("u_modelViewProjection", s_BatchData.CameraViewProjection);
        s_BatchData.DrawCalls++;
        RenderCommand::DrawIndexed(batch.VAO, batch.CurrentIndices, RendererAPI::PrimitiveType::Line);
    }

    template <typename VT>
    void Renderer2D::ResetBatch(BatchDataT<VT>& batch)
    {
        batch.CurrentVertices = 0;
        batch.CurrentIndices = 0;
        batch.CurrentTextureIndex = 0;
        batch.CurrentVertexPointer = reinterpret_cast<VT*>(batch.VerticesMemory);
        batch.CurrentIndexPointer = reinterpret_cast<U32*>(batch.IndicesMemory);
    }

    template <typename VT>
    void Renderer2D::ResetBatch(BatchDataLinesT<VT>& batch)
    {
        batch.CurrentVertices = 0;
        batch.CurrentIndices = 0;
        batch.CurrentVertexPointer = reinterpret_cast<VT*>(batch.VerticesMemory);
        batch.CurrentIndexPointer = reinterpret_cast<U32*>(batch.IndicesMemory);
    }

    template <typename VT>
    void Renderer2D::CheckForFlush(BatchDataT<VT>& batch, U32 verticesCount, U32 indicesCount)
    {
        if (batch.CurrentVertices + verticesCount > batch.MaxVertices || batch.CurrentIndices + indicesCount > batch.
            MaxIndices)
        {
            Flush(batch);
            ResetBatch(batch);
        }
    }

    template <typename VT>
    void Renderer2D::CheckForFlush(BatchDataLinesT<VT>& batch, U32 verticesCount, U32 indicesCount)
    {
        if (batch.CurrentVertices + verticesCount > batch.MaxVertices || batch.CurrentIndices + indicesCount > batch.
            MaxIndices)
        {
            Flush(batch);
            ResetBatch(batch);
        }
    }

    template <typename VT>
    void Renderer2D::PushVertex(BatchDataT<VT>& batch, const glm::vec2& referenceVertex,
                                const Component::Transform2D& transform, F32 depth, F32 textureIndex,
                                const glm::vec2& uv, const glm::vec4& tint,
                                const glm::vec2& tiling)
    {
        VT* vertex = batch.CurrentVertexPointer;
        vertex->Position = glm::vec3{referenceVertex, 0.0f};
        InitVertexGeometryData(*vertex, glm::vec3{transform.Position, depth}, transform.Scale, transform.Rotation);
        InitVertexColorData(*vertex, textureIndex, uv, tint, tiling);
        batch.CurrentVertexPointer = batch.CurrentVertexPointer + 1;
    }

    template <typename VT>
    void Renderer2D::PushVertex(BatchDataT<VT>& batch, const glm::vec2& referenceVertex, const glm::mat3& transform,
                                F32 depth, F32 textureIndex, const glm::vec2& uv, const glm::vec4& tint,
                                const glm::vec2& tiling)
    {
        VT* vertex = batch.CurrentVertexPointer;
        vertex->Position = transform * glm::vec3{referenceVertex, 1.0f} + glm::vec3{0.0f, 0.0f, depth};
        InitVertexColorData(*vertex, textureIndex, uv, tint, tiling);
        batch.CurrentVertexPointer = batch.CurrentVertexPointer + 1;
    }

    template <typename VT>
    void Renderer2D::InitVertexGeometryData(VT& vertex, const glm::vec3& position, const glm::vec2& scale,
                                            const glm::vec2& rotation)
    {
        vertex.Position *= glm::vec3{scale, 1.0f};
        vertex.Position = glm::vec3{
            rotation.x * vertex.Position.x - rotation.y * vertex.Position.y,
            rotation.y * vertex.Position.x + rotation.x * vertex.Position.y,
            0.0f
        };
        vertex.Position += position;
    }

    template <typename VT>
    void Renderer2D::InitVertexColorData(VT& vertex, F32 textureIndex, const glm::vec2& uv, const glm::vec4& tint,
                                         const glm::vec2& textureTiling)
    {
        vertex.TextureIndex = textureIndex;
        vertex.Color = tint;
        vertex.TextureTiling = textureTiling;
        vertex.UV = uv;
    }
}
