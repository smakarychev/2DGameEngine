#pragma once

#include "Engine/Core/Types.h"

#include "Buffer.h"
#include "RendererAPI.h"
#include "RenderQueue.h"
#include "Shader.h"
#include "SortingLayer.h"
#include "Texture.h"

#include "Engine/ECS/Components.h"
#include "Engine/Core/Camera.h"

#include <glm/glm.hpp>

namespace Engine
{
    using namespace Types;

    template <typename VerticesContainer>
    struct ShadingInfo
    {
        Texture* Texture = nullptr;
        VerticesContainer UV{};
        glm::vec4 Tint = glm::vec4{1.0f};
        glm::vec2 Tiling = glm::vec2{1.0f};
    };

    struct ShadingInfoCr
    {
        static ShadingInfo<std::array<glm::vec2, 4>> Create(const Component::SpriteRenderer& sr)
        {
            ShadingInfo<std::array<glm::vec2, 4>> sInfo{
                .Texture = sr.Texture,
                .UV = sr.UV,
                .Tint = sr.Tint,
                .Tiling = sr.Tiling
            };
            return sInfo;
        }

        static ShadingInfo<std::array<glm::vec2, 4>> Create(const Component::FontRenderer& fr, const char& character)
        {
            ShadingInfo<std::array<glm::vec2, 4>> sInfo{
                .Texture = &fr.Font->GetAtlas(),
                .UV = fr.Font->GetCharacters()[character].UV,
                .Tint = fr.Tint
            };
            return sInfo;
        }

        static ShadingInfo<std::vector<glm::vec2>> Create(const Component::PolygonRenderer& pr)
        {
            ShadingInfo<std::vector<glm::vec2>> sInfo{
                .Texture = pr.Texture,
                .UV = pr.Polygon->GetUVs(),
                .Tint = pr.Tint,
                .Tiling = pr.Tiling
            };
            return sInfo;
        }
    };

    template <typename Vertex>
    class BatchRenderer2D
    {
    private:
        struct Data
        {
            Ref<Shader> Shader;
            Ref<VertexArray> Vao;
            Camera* Camera = nullptr;

            RendererAPI::PrimitiveType Type = RendererAPI::PrimitiveType::Triangle;
            U32 MaxVertices = U32(2_MiB) / sizeof(Vertex);
            U32 MaxIndices = U32(2_MiB) / sizeof(U32);
            U32 MaxTextures = 32;
            Texture* UsedTextures[32] = {nullptr};
            U32 CurrentTextureIndex = 0;
            U32 CurrentVertices = 0;
            U32 CurrentIndices = 0;

            U8* VerticesMemory = nullptr;
            Vertex* CurrentVertexPointer = nullptr;
            U8* IndicesMemory = nullptr;
            U32* CurrentIndexPointer = nullptr;
        };

    public:
        // TODO: allow for predefined indices.
        void InitData(const std::string& shaderPath);
        void SetCamera(Camera* camera) { m_Data.Camera = camera; }
        void SetPrimitiveType(RendererAPI::PrimitiveType type) { m_Data.Type = type; }
        void Shutdown();
        void Flush();
        void Reset();
        bool ShouldFlush() const { return m_Data.CurrentVertices != 0; }
        template <typename Transform, typename VerticesContainer, typename IndicesContainer>
        void PushVertices(const Transform& transform,
                          F32 depth,
                          const ShadingInfo<VerticesContainer>& shadingInfo,
                          const VerticesContainer& referenceVertices,
                          const IndicesContainer& indices);
        template <typename Transform, typename VerticesContainer, typename IndicesContainer>
        void PushVertices(U32 entityId,
                          const Transform& transform,
                          F32 depth,
                          const ShadingInfo<VerticesContainer>& shadingInfo,
                          const VerticesContainer& referenceVertices,
                          const IndicesContainer& indices);
        template <typename VerticesContainer, typename IndicesContainer>
        void PushVertices(F32 depth,
                          const ShadingInfo<VerticesContainer>& shadingInfo,
                          const VerticesContainer& transformedVertices,
                          const IndicesContainer& indices);
        template <typename VerticesContainer, typename IndicesContainer>
        void PushVertices(U32 entityId,
                          F32 depth,
                          const ShadingInfo<VerticesContainer>& shadingInfo,
                          const VerticesContainer& transformedVertices,
                          const IndicesContainer& indices);

    private:
        void PushVertex(const glm::vec2& referenceVertex,
                        const Component::LocalToWorldTransform2D& transform,
                        F32 depth,
                        F32 textureIndex,
                        const glm::vec2& uv,
                        const glm::vec4& tint,
                        const glm::vec2& tiling);
        void PushVertex(const glm::vec2& referenceVertex,
                        const glm::mat3& transform,
                        F32 depth,
                        F32 textureIndex,
                        const glm::vec2& uv,
                        const glm::vec4& tint,
                        const glm::vec2& tiling);
        void PushVertex(const glm::vec2& transformedVertex,
                        F32 depth,
                        F32 textureIndex,
                        const glm::vec2& uv,
                        const glm::vec4& tint,
                        const glm::vec2& tiling);
        void CheckForFlush(U32 newVerticesCount, U32 newIndicesCount);
        F32 GetTextureIndex(Texture* texture);
        static void InitVertexGeometryData(Vertex& vertex,
                                           const glm::vec3& position,
                                           const glm::vec2& scale,
                                           const glm::vec2& rotation);
        static void InitVertexColorData(Vertex& vertex,
                                        F32 textureIndex,
                                        const glm::vec2& uv,
                                        const glm::vec4& tint,
                                        const glm::vec2& textureTiling = glm::vec2{1.0f});

    private:
        Data m_Data;
        bool m_IsInitialized = false;
    };

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::InitData(const std::string& shaderPath)
    {
        m_Data.Shader = Shader::ReadShaderFromFile(shaderPath);
        auto vbo = VertexBuffer::Create(nullptr, m_Data.MaxVertices * sizeof(Vertex));
        vbo->SetVertexLayout(Vertex::GetLayout());
        auto ibo = IndexBuffer::Create(nullptr, m_Data.MaxIndices);
        m_Data.Vao = VertexArray::Create();
        m_Data.Vao->AddVertexBuffer(vbo);
        m_Data.Vao->SetIndexBuffer(ibo);
        // Allocate memory for vertices / indices.

        m_Data.VerticesMemory = NewArr<U8>(m_Data.MaxVertices * sizeof(Vertex));
        m_Data.IndicesMemory = NewArr<U8>(m_Data.MaxIndices * sizeof(U32));
        m_Data.CurrentVertexPointer = reinterpret_cast<Vertex*>(m_Data.VerticesMemory);
        m_Data.CurrentIndexPointer = reinterpret_cast<U32*>(m_Data.IndicesMemory);
        m_IsInitialized = true;
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::Shutdown()
    {
        if (!m_IsInitialized) return;
        m_Data.Shader.reset();
        m_Data.Vao.reset();
        DeleteArr<Vertex>(reinterpret_cast<Vertex*>(m_Data.VerticesMemory), m_Data.MaxVertices);
        DeleteArr<U32>(reinterpret_cast<U32*>(m_Data.IndicesMemory), m_Data.MaxIndices);
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::Flush()
    {
        m_Data.Shader->Bind();
        m_Data.Vao->Bind();
        for (U32 i = 0; i < m_Data.CurrentTextureIndex; i++) m_Data.UsedTextures[i]->Bind(i);
        ENGINE_CORE_ASSERT(m_Data.Vao->GetVertexBuffers().size() == 1, "Batch must have only one vbo.")

        m_Data.Vao->GetVertexBuffers()[0]->SetData(m_Data.VerticesMemory, sizeof(Vertex) * m_Data.CurrentVertices);
        m_Data.Vao->GetIndexBuffer()->SetData(reinterpret_cast<U32*>(m_Data.IndicesMemory), m_Data.CurrentIndices);

        m_Data.Shader->SetUniformMat4("u_modelViewProjection", m_Data.Camera->GetViewProjection());
        RenderCommand::DrawIndexed(m_Data.Vao, m_Data.CurrentIndices, m_Data.Type);
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::Reset()
    {
        if (!m_IsInitialized) return;
        m_Data.CurrentVertices = 0;
        m_Data.CurrentIndices = 0;
        m_Data.CurrentTextureIndex = 0;
        m_Data.CurrentVertexPointer = reinterpret_cast<Vertex*>(m_Data.VerticesMemory);
        m_Data.CurrentIndexPointer = reinterpret_cast<U32*>(m_Data.IndicesMemory);
    }

    template <typename Vertex>
    template <typename Transform, typename VerticesContainer, typename IndicesContainer>
    void BatchRenderer2D<Vertex>::PushVertices(const Transform& transform,
                                               F32 depth,
                                               const ShadingInfo<VerticesContainer>& shadingInfo,
                                               const VerticesContainer& referenceVertices,
                                               const IndicesContainer& indices)
    {
        CheckForFlush((U32)referenceVertices.size(), (U32)indices.size());
        const F32 textureIndex = GetTextureIndex(shadingInfo.Texture);
        for (U32 i = 0; i < referenceVertices.size(); i++)
        {
            PushVertex(referenceVertices[i], transform, depth, textureIndex,
                       shadingInfo.UV[i], shadingInfo.Tint, shadingInfo.Tiling);
        }
        for (const auto i : indices)
        {
            U32* index = m_Data.CurrentIndexPointer;
            *index = i + m_Data.CurrentVertices;
            m_Data.CurrentIndexPointer = m_Data.CurrentIndexPointer + 1;
        }
        m_Data.CurrentVertices += U32(referenceVertices.size());
        m_Data.CurrentIndices += U32(indices.size());
    }

    template <typename Vertex>
    template <typename Transform, typename VerticesContainer, typename IndicesContainer>
    void BatchRenderer2D<Vertex>::PushVertices(U32 entityId,
                                               const Transform& transform,
                                               F32 depth,
                                               const ShadingInfo<VerticesContainer>& shadingInfo,
                                               const VerticesContainer& referenceVertices,
                                               const IndicesContainer& indices)
    {
        Vertex* firstVertex = m_Data.CurrentVertexPointer;
        PushVertices(transform, depth, shadingInfo, referenceVertices, indices);
        while (firstVertex < m_Data.CurrentVertexPointer)
        {
            firstVertex->EntityId = entityId;
            firstVertex = firstVertex + 1;
        }
    }

    template <typename Vertex>
    template <typename VerticesContainer, typename IndicesContainer>
    void BatchRenderer2D<Vertex>::PushVertices(F32 depth, const ShadingInfo<VerticesContainer>& shadingInfo,
                                               const VerticesContainer& transformedVertices,
                                               const IndicesContainer& indices)
    {
        CheckForFlush((U32)transformedVertices.size(), (U32)indices.size());
        const F32 textureIndex = GetTextureIndex(shadingInfo.Texture);
        for (U32 i = 0; i < transformedVertices.size(); i++)
        {
            PushVertex(transformedVertices[i], depth, textureIndex,
                       shadingInfo.UV[i], shadingInfo.Tint, shadingInfo.Tiling);
        }
        for (const auto i : indices)
        {
            U32* index = m_Data.CurrentIndexPointer;
            *index = i + m_Data.CurrentVertices;
            m_Data.CurrentIndexPointer = m_Data.CurrentIndexPointer + 1;
        }
        m_Data.CurrentVertices += U32(transformedVertices.size());
        m_Data.CurrentIndices += U32(indices.size());
    }

    template <typename Vertex>
    template <typename VerticesContainer, typename IndicesContainer>
    void BatchRenderer2D<Vertex>::PushVertices(U32 entityId, F32 depth,
                                               const ShadingInfo<VerticesContainer>& shadingInfo,
                                               const VerticesContainer& transformedVertices,
                                               const IndicesContainer& indices)
    {
        Vertex* firstVertex = m_Data.CurrentVertexPointer;
        PushVertices(depth, shadingInfo, transformedVertices, indices);
        while (firstVertex < m_Data.CurrentVertexPointer)
        {
            firstVertex->EntityId = entityId;
            firstVertex = firstVertex + 1;
        }
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::PushVertex(const glm::vec2& referenceVertex,
                                             const Component::LocalToWorldTransform2D& transform,
                                             F32 depth,
                                             F32 textureIndex,
                                             const glm::vec2& uv,
                                             const glm::vec4& tint,
                                             const glm::vec2& tiling)
    {
        Vertex* vertex = m_Data.CurrentVertexPointer;
        vertex->Position = glm::vec3{referenceVertex, 0.0f};
        InitVertexGeometryData(*vertex, glm::vec3{transform.Position, depth}, transform.Scale, transform.Rotation);
        InitVertexColorData(*vertex, textureIndex, uv, tint, tiling);
        m_Data.CurrentVertexPointer = m_Data.CurrentVertexPointer + 1;
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::PushVertex(const glm::vec2& referenceVertex,
                                             const glm::mat3& transform,
                                             F32 depth,
                                             F32 textureIndex,
                                             const glm::vec2& uv,
                                             const glm::vec4& tint,
                                             const glm::vec2& tiling)
    {
        Vertex* vertex = m_Data.CurrentVertexPointer;
        vertex->Position = transform * glm::vec3{referenceVertex, 1.0f} + glm::vec3{0.0f, 0.0f, depth};
        InitVertexColorData(*vertex, textureIndex, uv, tint, tiling);
        m_Data.CurrentVertexPointer = m_Data.CurrentVertexPointer + 1;
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::PushVertex(const glm::vec2& transformedVertex, F32 depth, F32 textureIndex,
                                             const glm::vec2& uv, const glm::vec4& tint, const glm::vec2& tiling)
    {
        Vertex* vertex = m_Data.CurrentVertexPointer;
        vertex->Position = glm::vec3{transformedVertex, depth};
        InitVertexColorData(*vertex, textureIndex, uv, tint, tiling);
        m_Data.CurrentVertexPointer = m_Data.CurrentVertexPointer + 1;
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::CheckForFlush(U32 newVerticesCount, U32 newIndicesCount)
    {
        if (m_Data.CurrentVertices + newVerticesCount > m_Data.MaxVertices ||
            m_Data.CurrentIndices + newIndicesCount > m_Data.MaxIndices)
        {
            Flush();
            Reset();
        }
    }

    template <typename Vertex>
    F32 BatchRenderer2D<Vertex>::GetTextureIndex(Texture* texture)
    {
        F32 textureIndex = -1.0f;
        if (texture != nullptr)
        {
            bool textureFound = false;
            for (U32 i = 0; i < m_Data.CurrentTextureIndex; i++)
            {
                if (m_Data.UsedTextures[i] == texture)
                {
                    textureIndex = F32(i);
                    textureFound = true;
                    break;
                }
            }
            // If no such texture in the batch.
            if (textureFound == false)
            {
                if (m_Data.CurrentTextureIndex >= m_Data.MaxTextures)
                {
                    Flush();
                    Reset();
                }
                m_Data.UsedTextures[m_Data.CurrentTextureIndex] = texture;
                textureIndex = F32(m_Data.CurrentTextureIndex);
                m_Data.CurrentTextureIndex = m_Data.CurrentTextureIndex + 1;
            }
        }
        return textureIndex;
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::InitVertexGeometryData(Vertex& vertex, const glm::vec3& position,
                                                         const glm::vec2& scale, const glm::vec2& rotation)
    {
        vertex.Position *= glm::vec3{scale, 1.0f};
        vertex.Position = glm::vec3{
            rotation.x * vertex.Position.x - rotation.y * vertex.Position.y,
            rotation.y * vertex.Position.x + rotation.x * vertex.Position.y,
            0.0f
        };
        vertex.Position += position;
    }

    template <typename Vertex>
    void BatchRenderer2D<Vertex>::InitVertexColorData(Vertex& vertex, F32 textureIndex, const glm::vec2& uv,
                                                      const glm::vec4& tint, const glm::vec2& textureTiling)
    {
        vertex.TextureIndex = textureIndex;
        vertex.Color = tint;
        vertex.TextureTiling = textureTiling;
        vertex.UV = uv;
    }

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
            glm::vec3 Position = glm::vec3{0.0f};
            F32 TextureIndex = -1.0f;
            glm::vec2 UV = glm::vec2{0.0f};
            glm::vec2 TextureTiling = glm::vec2{1.0f};
            glm::vec4 Color = glm::vec4{1.0f};
            U32 EntityId = std::numeric_limits<U32>::max();

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
                        {LayoutElement::UInt, "a_entityId"},
                    }
                };
                return layout;
            }

            BatchVertexEditor(U32 entityId, const glm::vec3& pos, const glm::vec2& uv, const glm::vec4 color) :
                Position(pos), UV(uv), Color(color), EntityId(entityId)
            {
            }

            BatchVertexEditor(U32 entityId, const glm::vec3& pos, const glm::vec2& uv, const Texture& texture,
                              const glm::vec2& textureTiling = glm::vec2{1.0f}) :
                Position(pos), TextureIndex(F32(texture.GetId())), UV(uv), TextureTiling(textureTiling),
                EntityId(entityId)
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
            U32 EntityId = std::numeric_limits<U32>::max();
            glm::vec3 Position = glm::vec3{0.0f};
            glm::vec4 Color = glm::vec4{1.0f};

            static const VertexLayout& GetLayout()
            {
                const static VertexLayout layout{
                    {
                        {LayoutElement::Float3, "a_position"},
                        {LayoutElement::Float4, "a_color"},
                        {LayoutElement::UInt, "a_entityId"},
                    }
                };
                return layout;
            }

            BatchVertexLineEditor(U32 entityId, const glm::vec3& pos, const glm::vec4 color) :
                EntityId(entityId), Position(pos), Color(color)
            {
            }

            BatchVertexLineEditor() = default;
        };

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
                std::array<glm::vec2, 4> Position;
                std::array<U32, 6> Indices;
            };

            ReferenceQuad ReferenceQuad{};

            BatchRenderer2D<BatchVertex> TriangleBatch;
            BatchRenderer2D<BatchVertexLine> LineBatch;
            BatchRenderer2D<BatchVertex> TextBatch;

            BatchRenderer2D<BatchVertexEditor> TriangleBatchEditor;
            BatchRenderer2D<BatchVertexLineEditor> LineBatchEditor;
            BatchRenderer2D<BatchVertexEditor> TextBatchEditor;

            SortingLayer* SortingLayer = &DefaultSortingLayer;
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

        static void SetSortingLayer(const SortingLayer& layer) { *s_BatchData.SortingLayer = layer; }
        static const SortingLayer& GetSortingLayer() { return *s_BatchData.SortingLayer; }

        static void BeginScene(Camera* camera, SortingLayer* sortingLayer = &DefaultSortingLayer);
        static void EndScene();
        static void Reset();

        static void DrawQuad(const Component::LocalToWorldTransform2D& transform,
                             const Component::SpriteRenderer& spriteRenderer,
                             RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawQuad(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                             RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygon(const Component::LocalToWorldTransform2D& transform,
                                const Component::PolygonRenderer& polygonRenderer,
                                RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygon(const glm::mat3& transform, const Component::PolygonRenderer& polygonRenderer,
                                RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        // WinApi uses DrawText as a macro >:(
        // `fontSize` as if camera is exactly one unit away.
        static void DrawFont(const Component::FontRenderer& fontRenderer, const std::string& text);
        // Fixed font retains its relative position and size as camera moves/zooms.
        static void DrawFontFixed(const Component::FontRenderer& fontRenderer, const std::string& text);
        static void DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color = glm::vec4{1.0f},
                             F32 depth = 0.0f);

        static void DrawQuadEditor(U32 entityId, const Component::LocalToWorldTransform2D& transform,
                                   const Component::SpriteRenderer& spriteRenderer,
                                   RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawQuadEditor(U32 entityId, const glm::mat3& transform,
                                   const Component::SpriteRenderer& spriteRenderer,
                                   RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonEditor(U32 entityId, const Component::LocalToWorldTransform2D& transform,
                                      const Component::PolygonRenderer& polygonRenderer,
                                      RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonEditor(U32 entityId, const glm::mat3& transform,
                                      const Component::PolygonRenderer& polygonRenderer,
                                      RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawFontEditor(U32 entityId, const Component::FontRenderer& fontRenderer, const std::string& text);
        static void DrawFontFixedEditor(U32 entityId, const Component::FontRenderer& fontRenderer,
                                        const std::string& text);
        static void DrawLineEditor(U32 entityId, const glm::vec2& from, const glm::vec2& to,
                                   const glm::vec4& color = glm::vec4{1.0f}, F32 depth = 0.0f);

    private:
        // These functions are to be executed by a command queue (currently unimplemented).

        static void DrawQuadCall(const Component::LocalToWorldTransform2D& transform,
                                 const Component::SpriteRenderer& spriteRenderer, F32 depth = 0.0f,
                                 RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawQuadCall(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                                 F32 depth = 0.0f,
                                 RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonCall(const Component::LocalToWorldTransform2D& transform,
                                    const Component::PolygonRenderer& polygonRenderer, F32 depth = 0.0f,
                                    RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonCall(const glm::mat3& transform2D, const Component::PolygonRenderer& polygonRenderer,
                                    F32 depth = 0.0f,
                                    RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawFontCall(const Component::FontRenderer& fontRenderer, const std::string& text,
                                 F32 depth = 0.0f);
        static void DrawFontFixedCall(const Component::FontRenderer& fontRenderer, const std::string& text);
        static void DrawLineCall(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color, F32 depth = 0.0f);

        static void DrawQuadEditorCall(U32 entityId, const Component::LocalToWorldTransform2D& transform,
                                       const Component::SpriteRenderer& spriteRenderer, F32 depth = 0.0f,
                                       RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawQuadEditorCall(U32 entityId, const glm::mat3& transform,
                                       const Component::SpriteRenderer& spriteRenderer, F32 depth = 0.0f,
                                       RendererAPI::PrimitiveType primitiveType = RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonEditorCall(U32 entityId, const Component::LocalToWorldTransform2D& transform,
                                          const Component::PolygonRenderer& polygonRenderer, F32 depth = 0.0f,
                                          RendererAPI::PrimitiveType primitiveType =
                                              RendererAPI::PrimitiveType::Triangle);
        static void DrawPolygonEditorCall(U32 entityId, const glm::mat3& transform,
                                          const Component::PolygonRenderer& polygonRenderer, F32 depth = 0.0f,
                                          RendererAPI::PrimitiveType primitiveType =
                                              RendererAPI::PrimitiveType::Triangle);
        static void DrawFontEditorCall(U32 entityId, const Component::FontRenderer& fontRenderer,
                                       const std::string& text, F32 depth = 0.0f);
        static void DrawFontFixedEditorCall(U32 entityId, const Component::FontRenderer& fontRenderer,
                                            const std::string& text);
        static void DrawLineEditorCall(U32 entityId, const glm::vec2& from, const glm::vec2& to, const glm::vec4& color,
                                       F32 depth = 0.0f);

        static void DrawOutlineCall(const Component::LocalToWorldTransform2D& transform,
                                    const Component::SpriteRenderer& spriteRenderer, F32 depth = 0.0f);
        static void DrawOutlineCall(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                                    F32 depth = 0.0f);
        static void DrawOutlineCall(const Component::LocalToWorldTransform2D& transform,
                                    const Component::PolygonRenderer& polygonRenderer, F32 depth = 0.0f);
        static void DrawOutlineCall(const glm::mat3& transform,
                                    const Component::PolygonRenderer& polygonRenderer, F32 depth = 0.0f);

        static Component::LocalToWorldTransform2D FlipTransformIfNeeded(
            const Component::LocalToWorldTransform2D& transform, const Component::SpriteRenderer& spriteRenderer);
        static glm::mat3 FlipTransformIfNeeded(const glm::mat3& transform,
                                               const Component::SpriteRenderer& spriteRenderer);

    private:
        static BatchRendererData s_BatchData;
    };

    template <>
    inline void BatchRenderer2D<Renderer2D::BatchVertexLine>::InitVertexColorData(
        Renderer2D::BatchVertexLine& vertex, F32 textureIndex, const glm::vec2& uv,
        const glm::vec4& tint, const glm::vec2& textureTiling)
    {
        vertex.Color = tint;
    }

    template <>
    inline void BatchRenderer2D<Renderer2D::BatchVertexLineEditor>::InitVertexColorData(
        Renderer2D::BatchVertexLineEditor& vertex, F32 textureIndex, const glm::vec2& uv,
        const glm::vec4& tint, const glm::vec2& textureTiling)
    {
        vertex.Color = tint;
    }
}
