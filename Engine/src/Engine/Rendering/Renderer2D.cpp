#include "enginepch.h"

#include "Renderer2D.h"
#include "RenderCommand.h"

#include "Engine/Core/Core.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
    Renderer2D::BatchRendererData Renderer2D::s_BatchData;

    void Renderer2D::Init()
    {
        // Initialize batch shader and batch buffers.
        s_BatchData.BatchShader = Shader::ReadShaderFromFile("assets/shaders/batchShader.glsl");
        s_BatchData.TextShader = Shader::ReadShaderFromFile("assets/shaders/textShader.glsl");
        s_BatchData.LineShader = Shader::ReadShaderFromFile("assets/shaders/lineShader.glsl");

        //*************** Init lines *********************************************************
        {
            BatchDataLines lineBatch;
            auto vbo = VertexBuffer::Create(nullptr, lineBatch.MaxVertices * sizeof(BatchVertexLine));
            vbo->SetVertexLayout(BatchVertexLine::GetLayout());
            U32* indices = NewArr<U32>(lineBatch.MaxIndices);
            for (U32 i = 0; i + 2 < lineBatch.MaxIndices; i += 2)
            {
                indices[i] = i;
                indices[i + 1] = i + 1;
            }
            auto ibo = IndexBuffer::Create(indices, lineBatch.MaxIndices);
            lineBatch.VAO = VertexArray::Create();
            lineBatch.VAO->AddVertexBuffer(vbo);
            lineBatch.VAO->SetIndexBuffer(ibo);

            lineBatch.VerticesMemory = NewArr<U8>(lineBatch.MaxVertices * sizeof(BatchVertexLine));
            lineBatch.CurrentVertexPointer = reinterpret_cast<BatchVertexLine*>(lineBatch.VerticesMemory);
            s_BatchData.LineBatch = lineBatch;

            DeleteArr<U32>(indices, lineBatch.MaxIndices);
        }
        //*************** Init quad **********************************************************
        {
            s_BatchData.ReferenceQuad.Position[0] = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
            s_BatchData.ReferenceQuad.Position[1] = glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);
            s_BatchData.ReferenceQuad.Position[2] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
            s_BatchData.ReferenceQuad.Position[3] = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);

            s_BatchData.ReferenceQuad.UV[0] = glm::vec2(0.0f, 0.0f);
            s_BatchData.ReferenceQuad.UV[1] = glm::vec2(1.0f, 0.0f);
            s_BatchData.ReferenceQuad.UV[2] = glm::vec2(1.0f, 1.0f);
            s_BatchData.ReferenceQuad.UV[3] = glm::vec2(0.0f, 1.0f);

            // Indices structure is predictable.
            BatchData quadBatch;
            U32* indices = NewArr<U32>(quadBatch.MaxIndices);
            for (U32 i = 0; i + 6 < quadBatch.MaxIndices; i += 6)
            {
                U32 currVer = quadBatch.CurrentVertices;
                indices[i] = currVer + 0;
                indices[i + 1] = currVer + 1;
                indices[i + 2] = currVer + 3;
                indices[i + 3] = currVer + 1;
                indices[i + 4] = currVer + 2;
                indices[i + 5] = currVer + 3;
                quadBatch.CurrentVertices += 4;
            }
            quadBatch.CurrentVertices = 0;

            auto vbo = VertexBuffer::Create(nullptr, quadBatch.MaxVertices * sizeof(BatchVertex));
            vbo->SetVertexLayout(BatchVertex::GetLayout());
            auto ibo = IndexBuffer::Create(indices, quadBatch.MaxIndices);
            quadBatch.VAO = VertexArray::Create();
            quadBatch.VAO->AddVertexBuffer(vbo);
            quadBatch.VAO->SetIndexBuffer(ibo);

            quadBatch.VerticesMemory = NewArr<U8>(quadBatch.MaxVertices * sizeof(BatchVertex));
            quadBatch.CurrentVertexPointer = reinterpret_cast<BatchVertex*>(quadBatch.VerticesMemory);
            s_BatchData.QuadBatch = quadBatch;

            DeleteArr<U32>(indices, quadBatch.MaxIndices);
        }
        //*************** Init regular polygon ***********************************************
        {
            BatchData polygonBatch;
            auto vbo = VertexBuffer::Create(nullptr, polygonBatch.MaxVertices * sizeof(BatchVertex));
            vbo->SetVertexLayout(BatchVertex::GetLayout());
            auto ibo = IndexBuffer::Create(nullptr, polygonBatch.MaxIndices);
            polygonBatch.VAO = VertexArray::Create();
            polygonBatch.VAO->AddVertexBuffer(vbo);
            polygonBatch.VAO->SetIndexBuffer(ibo);

            polygonBatch.VerticesMemory = NewArr<U8>(polygonBatch.MaxVertices * sizeof(BatchVertex));
            polygonBatch.CurrentVertexPointer = reinterpret_cast<BatchVertex*>(polygonBatch.VerticesMemory);
            polygonBatch.IndicesMemory = NewArr<U8>(polygonBatch.MaxIndices * sizeof(U32));
            polygonBatch.CurrentIndexPointer = reinterpret_cast<U32*>(polygonBatch.IndicesMemory);
            s_BatchData.PolygonBatch = polygonBatch;
        }
        //*************** Init text **********************************************************
        {
            BatchData textBatch;
            U32* indices = NewArr<U32>(textBatch.MaxIndices);
            for (U32 i = 0; i + 6 < textBatch.MaxIndices; i += 6)
            {
                U32 currVer = textBatch.CurrentVertices;
                indices[i] = currVer + 0;
                indices[i + 1] = currVer + 1;
                indices[i + 2] = currVer + 3;
                indices[i + 3] = currVer + 1;
                indices[i + 4] = currVer + 2;
                indices[i + 5] = currVer + 3;
                textBatch.CurrentVertices += 4;
            }
            textBatch.CurrentVertices = 0;

            auto vbo = VertexBuffer::Create(nullptr, textBatch.MaxVertices * sizeof(BatchVertex));
            vbo->SetVertexLayout(BatchVertex::GetLayout());
            auto ibo = IndexBuffer::Create(indices, textBatch.MaxIndices);
            textBatch.VAO = VertexArray::Create();
            textBatch.VAO->AddVertexBuffer(vbo);
            textBatch.VAO->SetIndexBuffer(ibo);

            textBatch.VerticesMemory = NewArr<U8>(textBatch.MaxVertices * sizeof(BatchVertex));
            textBatch.CurrentVertexPointer = reinterpret_cast<BatchVertex*>(textBatch.VerticesMemory);
            s_BatchData.TextBatch = textBatch;

            DeleteArr<U32>(indices, textBatch.MaxIndices);
        }
    }

    void Renderer2D::BeginScene(Ref<Camera> camera, SortingLayer* sortingLayer)
    {
        s_BatchData.CameraViewProjection = camera->GetViewProjection();
        s_BatchData.SortingLayer = sortingLayer;
        s_BatchData.Camera = camera.get();
        s_BatchData.RenderQueue.Clear();
    }

    void Renderer2D::EndScene()
    {
        RenderCommand::EnableCull(false);

        //s_BatchData.RenderQueue.Execute();
        // Lines are rendered first because they are less likely to be transparent.
        // (transparency is only partially supported yet).
        if (s_BatchData.LineBatch.CurrentIndices > 0)
        {
            Flush(s_BatchData.LineBatch);
            ResetBatch(s_BatchData.LineBatch);
        }

        if (s_BatchData.QuadBatch.CurrentIndices > 0)
        {
            Flush(s_BatchData.QuadBatch);
            ResetBatch(s_BatchData.QuadBatch);
        }
        if (s_BatchData.PolygonBatch.CurrentIndices > 0)
        {
            Flush(s_BatchData.PolygonBatch);
            ResetBatch(s_BatchData.PolygonBatch);
        }
        if (s_BatchData.TextBatch.CurrentIndices > 0)
        {
            Flush(s_BatchData.TextBatch, *s_BatchData.TextShader);
            ResetBatch(s_BatchData.TextBatch);
        }
        //ENGINE_INFO("Renderer2D total draw calls: {}", s_BatchData.DrawCalls);
        s_BatchData.DrawCalls = 0;
    }

    void Renderer2D::DrawQuad(const Component::Transform2D& transform, const Component::SpriteRenderer& spriteRenderer,
                              RendererAPI::PrimitiveType primitiveType)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(spriteRenderer.SortingLayer,
                                                                        spriteRenderer.OrderInLayer);
        DrawQuadCall(transform, spriteRenderer, depth, primitiveType);
    }

    void Renderer2D::DrawQuad(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                              RendererAPI::PrimitiveType primitiveType)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(spriteRenderer.SortingLayer,
                                                                        spriteRenderer.OrderInLayer);
        DrawQuadCall(transform, spriteRenderer, depth, primitiveType);
    }

    void Renderer2D::DrawPolygon(const Component::Transform2D& transform,
                                 const Component::PolygonRenderer& polygonRenderer)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(polygonRenderer.SortingLayer,
                                                                        polygonRenderer.OrderInLayer);
        DrawPolygonCall(transform, polygonRenderer, depth);
    }

    void Renderer2D::DrawPolygon(const glm::mat3& transform, const Component::PolygonRenderer& polygonRenderer)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(polygonRenderer.SortingLayer,
                                                                        polygonRenderer.OrderInLayer);
        DrawPolygonCall(transform, polygonRenderer, depth);
    }

    void Renderer2D::DrawFont(const Component::FontRenderer& fontRenderer, const std::string& text)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(fontRenderer.SortingLayer,
                                                                        fontRenderer.OrderInLayer);
        DrawFontCall(fontRenderer, text, depth);
    }

    void Renderer2D::DrawFontFixed(const Component::FontRenderer& fontRenderer, const std::string& text)
    {
        DrawFontFixedCall(fontRenderer, text);
    }

    void Renderer2D::DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color)
    {
        DrawLineCall(from, to, color, 0.0f);
    }

    void Renderer2D::DrawQuadEditor(I32 entityId, const Component::Transform2D& transform,
                                    const Component::SpriteRenderer& spriteRenderer,
                                    RendererAPI::PrimitiveType primitiveType)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(spriteRenderer.SortingLayer,
                                                                        spriteRenderer.OrderInLayer);
        DrawQuadEditorCall(entityId, transform, spriteRenderer, depth, primitiveType);
    }

    void Renderer2D::DrawQuadEditor(I32 entityId, const glm::mat3& transform,
                                    const Component::SpriteRenderer& spriteRenderer,
                                    RendererAPI::PrimitiveType primitiveType)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(spriteRenderer.SortingLayer,
                                                                        spriteRenderer.OrderInLayer);
        DrawQuadEditorCall(entityId, transform, spriteRenderer, depth, primitiveType);
    }

    void Renderer2D::DrawPolygonEditor(I32 entityId, const Component::Transform2D& transform,
                                       const Component::PolygonRenderer& polygonRenderer)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(polygonRenderer.SortingLayer,
                                                                        polygonRenderer.OrderInLayer);
        DrawPolygonEditorCall(entityId, transform, polygonRenderer, depth);
    }

    void Renderer2D::DrawPolygonEditor(I32 entityId, const glm::mat3& transform,
                                       const Component::PolygonRenderer& polygonRenderer)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(polygonRenderer.SortingLayer,
                                                                        polygonRenderer.OrderInLayer);
        DrawPolygonEditorCall(entityId, transform, polygonRenderer, depth);
    }

    void Renderer2D::DrawFontEditor(I32 entityId, const Component::FontRenderer& fontRenderer, const std::string& text)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(fontRenderer.SortingLayer,
                                                                        fontRenderer.OrderInLayer);
        DrawFontEditorCall(entityId, fontRenderer, text, depth);
    }

    void Renderer2D::DrawFontFixedEditor(I32 entityId, const Component::FontRenderer& fontRenderer,
                                         const std::string& text)
    {
        DrawFontFixedEditorCall(entityId, fontRenderer, text);
    }

    void Renderer2D::DrawLineEditor(I32 entityId, const glm::vec2& from, const glm::vec2& to, const glm::vec4& color)
    {
        DrawLineEditorCall(entityId, from, to, color, 0.0f);
    }

    void Renderer2D::DrawQuadCall(const Component::Transform2D& transform,
                                  const Component::SpriteRenderer& spriteRenderer, F32 depth,
                                  RendererAPI::PrimitiveType primitiveType)
    {
        if (primitiveType == RendererAPI::PrimitiveType::Line)
        {
            DrawOutlineCall(transform, spriteRenderer, depth);
            return;
        }
        BatchData& quadBatch = s_BatchData.QuadBatch;
        CheckForFlush(quadBatch, 4, 6);
        const F32 textureIndex = GetTextureIndex(quadBatch, spriteRenderer.Texture);
        const auto& referenceQuad = s_BatchData.ReferenceQuad;

        // Create new quad.
        for (U32 i = 0; i < 4; i++)
        {
            PushVertex(quadBatch, referenceQuad.Position[i], transform, depth, textureIndex,
                       spriteRenderer.UV[i], spriteRenderer.Tint, spriteRenderer.Tiling);
        }
        quadBatch.CurrentVertices += 4;
        quadBatch.CurrentIndices += 6;
    }

    void Renderer2D::DrawQuadCall(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                                  F32 depth, RendererAPI::PrimitiveType primitiveType)
    {
        if (primitiveType == RendererAPI::PrimitiveType::Line)
        {
            DrawOutlineCall(transform, spriteRenderer, depth);
            return;
        }
        BatchData& quadBatch = s_BatchData.QuadBatch;
        CheckForFlush(quadBatch, 4, 6);
        const F32 textureIndex = GetTextureIndex(quadBatch, spriteRenderer.Texture);
        const auto& referenceQuad = s_BatchData.ReferenceQuad;

        // Create new quad.
        for (U32 i = 0; i < 4; i++)
        {
            PushVertex(quadBatch, referenceQuad.Position[i], transform, depth, textureIndex,
                       spriteRenderer.UV[i], spriteRenderer.Tint, spriteRenderer.Tiling);
        }
        quadBatch.CurrentVertices += 4;
        quadBatch.CurrentIndices += 6;
    }

    void Renderer2D::DrawPolygonCall(const Component::Transform2D& transform,
                                     const Component::PolygonRenderer& polygonRenderer, F32 depth)
    {
        BatchData& polygonBatch = s_BatchData.PolygonBatch;
        const RegularPolygon& polygon = *polygonRenderer.Polygon;
        CheckForFlush(polygonBatch, polygon.GetNumberOfVertices(), polygon.GetNumberOfIndices());
        const F32 textureIndex = GetTextureIndex(polygonBatch, polygonRenderer.Texture);

        for (U32 i = 0; i < polygon.GetNumberOfVertices(); i++)
        {
            PushVertex(polygonBatch, polygon.GetVertices()[i], transform, depth, textureIndex,
                       polygon.GetUVs()[i], polygonRenderer.Tint, polygonRenderer.Tiling);
        }
        for (const auto i : polygon.GetIndices())
        {
            U32* index = polygonBatch.CurrentIndexPointer;
            *index = i + polygonBatch.CurrentVertices;
            polygonBatch.CurrentIndexPointer++;
        }
        polygonBatch.CurrentVertices += U32(polygon.GetVertices().size());
        polygonBatch.CurrentIndices += U32(polygon.GetIndices().size());
    }

    void Renderer2D::DrawPolygonCall(const glm::mat3& transform, const Component::PolygonRenderer& polygonRenderer,
                                     F32 depth)
    {
        BatchData& polygonBatch = s_BatchData.PolygonBatch;
        const RegularPolygon& polygon = *polygonRenderer.Polygon;
        CheckForFlush(polygonBatch, polygon.GetNumberOfVertices(), polygon.GetNumberOfIndices());
        const F32 textureIndex = GetTextureIndex(polygonBatch, polygonRenderer.Texture);

        for (U32 i = 0; i < polygon.GetNumberOfVertices(); i++)
        {
            PushVertex(polygonBatch, polygon.GetVertices()[i], transform, depth, textureIndex,
                       polygon.GetUVs()[i], polygonRenderer.Tint, polygonRenderer.Tiling);
        }
        for (const auto i : polygon.GetIndices())
        {
            U32* index = polygonBatch.CurrentIndexPointer;
            *index = i + polygonBatch.CurrentVertices;
            polygonBatch.CurrentIndexPointer++;
        }
        polygonBatch.CurrentVertices += U32(polygon.GetVertices().size());
        polygonBatch.CurrentIndices += U32(polygon.GetIndices().size());
    }

    void Renderer2D::DrawFontCall(const Component::FontRenderer& fontRenderer, const std::string& text, F32 depth)
    {
        BatchData& textBatch = s_BatchData.TextBatch;
        CheckForFlush(textBatch, 4, 6);
        const Font& font = *fontRenderer.Font;
        F32 textureIndex = GetTextureIndex(textBatch, &font.GetAtlas());
        const auto& referenceQuad = s_BatchData.ReferenceQuad;

        F32 fontSizeCoeff = fontRenderer.FontSize / font.GetBaseFontSize() * s_BatchData.Camera->
            GetPixelCoefficient(1.0f);
        F32 x = fontRenderer.FontRect.Min.x;
        F32 y = fontRenderer.FontRect.Min.y;

        for (auto& ch : text)
        {
            if (ch == ' ')
            {
                x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
                continue;
            }

            if (x + font.GetCharacters()[ch].Size.x * fontSizeCoeff > fontRenderer.FontRect.Min.x)
            {
                y -= fontRenderer.Font->GetLineHeight() * fontSizeCoeff;
                x = fontRenderer.FontRect.Min.x;
            }
            // Create new quad.
            for (U32 i = 0; i < 4; i++)
            {
                Component::Transform2D transform{
                    glm::vec2{x, y} + font.GetCharacters()[ch].Bearing * fontSizeCoeff,
                    glm::vec2{font.GetCharacters()[ch].Size * fontSizeCoeff},
                    glm::vec2{1.0f, 0.0f}
                };
                PushVertex(textBatch, referenceQuad.Position[i], transform,
                           depth,
                           textureIndex,
                           fontRenderer.Font->GetCharacters()[ch].UV[i],
                           fontRenderer.Tint,
                           glm::vec2{1.0f});
            }
            textBatch.CurrentVertices += 4;
            textBatch.CurrentIndices += 6;
            x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
        }
    }

    void Renderer2D::DrawFontFixedCall(const Component::FontRenderer& fontRenderer, const std::string& text)
    {
        BatchData& textBatch = s_BatchData.TextBatch;
        CheckForFlush(textBatch, 4, 6);
        const Font& font = *fontRenderer.Font;
        F32 textureIndex = GetTextureIndex(textBatch, &font.GetAtlas());
        const auto& referenceQuad = s_BatchData.ReferenceQuad;

        F32 fontSizeCoeff = fontRenderer.FontSize / font.GetBaseFontSize() * s_BatchData.Camera->GetPixelCoefficient();
        F32 xminPx = fontRenderer.FontRect.Min.x;
        F32 xmaxPx = fontRenderer.FontRect.Max.x;
        F32 yminPx = fontRenderer.FontRect.Min.y;
        xminPx -= (F32)s_BatchData.Camera->GetViewportWidth() / 2.0f;
        xmaxPx -= (F32)s_BatchData.Camera->GetViewportWidth() / 2.0f;
        yminPx = (F32)s_BatchData.Camera->GetViewportHeight() / 2.0f - fontRenderer.FontSize - yminPx;
        xminPx *= s_BatchData.Camera->GetPixelCoefficient();
        xmaxPx *= s_BatchData.Camera->GetPixelCoefficient();
        yminPx *= s_BatchData.Camera->GetPixelCoefficient();
        F32 x = xminPx;
        F32 y = yminPx;

        for (auto& ch : text)
        {
            if (ch == ' ')
            {
                x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
                continue;
            }

            if (x + font.GetCharacters()[ch].Size.x * fontSizeCoeff > xmaxPx)
            {
                y -= font.GetLineHeight() * fontSizeCoeff;
                x = xminPx;
            }
            // Create new quad.
            for (U32 i = 0; i < 4; i++)
            {
                Component::Transform2D transform{
                    glm::vec2{x, y} + font.GetCharacters()[ch].Bearing * fontSizeCoeff + glm::vec2{
                        s_BatchData.Camera->GetPosition()
                    },
                    glm::vec2{font.GetCharacters()[ch].Size * fontSizeCoeff},
                    glm::vec2{1.0f, 0.0f}
                };
                PushVertex(textBatch, referenceQuad.Position[i], transform,
                           1.0f,
                           textureIndex,
                           fontRenderer.Font->GetCharacters()[ch].UV[i],
                           fontRenderer.Tint,
                           glm::vec2{1.0f});
            }
            textBatch.CurrentVertices += 4;
            textBatch.CurrentIndices += 6;
            x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
        }
    }

    void Renderer2D::DrawLineCall(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color, F32 depth)
    {
        BatchDataLines& lineBatch = s_BatchData.LineBatch;
        CheckForFlush(lineBatch, 2, 2);
        // Create new line.
        BatchVertexLine* vertex = lineBatch.CurrentVertexPointer;
        vertex->Position = glm::vec3{from.x, from.y, depth};
        vertex->Color = color;
        lineBatch.CurrentVertexPointer++;
        vertex = lineBatch.CurrentVertexPointer;
        vertex->Position = glm::vec3{to.x, to.y, depth};
        vertex->Color = color;
        lineBatch.CurrentVertexPointer++;

        lineBatch.CurrentVertices += 2;
        lineBatch.CurrentIndices += 2;
    }

    void Renderer2D::DrawQuadEditorCall(I32 entityId, const Component::Transform2D& transform,
                                        const Component::SpriteRenderer& spriteRenderer, F32 depth,
                                        RendererAPI::PrimitiveType primitiveType)
    {
    }

    void Renderer2D::DrawQuadEditorCall(I32 entityId, const glm::mat3& transform,
                                        const Component::SpriteRenderer& spriteRenderer, F32 depth,
                                        RendererAPI::PrimitiveType primitiveType)
    {
    }

    void Renderer2D::DrawPolygonEditorCall(I32 entityId, const Component::Transform2D& transform,
                                           const Component::PolygonRenderer& polygonRenderer, F32 depth)
    {
    }

    void Renderer2D::DrawPolygonEditorCall(I32 entityId, const glm::mat3& transform,
                                           const Component::PolygonRenderer& polygonRenderer, F32 depth)
    {
    }

    void Renderer2D::DrawFontEditorCall(I32 entityId, const Component::FontRenderer& fontRenderer,
                                        const std::string& text, F32 depth)
    {
    }

    void Renderer2D::DrawFontFixedEditorCall(I32 entityId, const Component::FontRenderer& fontRenderer,
                                             const std::string& text)
    {
    }

    void Renderer2D::DrawLineEditorCall(I32 entityId, const glm::vec2& from, const glm::vec2& to,
                                        const glm::vec4& color, F32 depth)
    {
    }

    void Renderer2D::DrawOutlineCall(const Component::Transform2D& transform,
                                     const Component::SpriteRenderer& spriteRenderer, F32 depth)
    {
        auto& referenceQuad = s_BatchData.ReferenceQuad;
        BatchVertex vertices[4];
        // Create new quad.
        for (U32 i = 0; i < 4; i++)
        {
            BatchVertex& vertex = vertices[i];
            vertex.Position = glm::vec3(referenceQuad.Position[i]);
            InitVertexGeometryData(vertex, glm::vec3{transform.Position, depth}, transform.Scale, transform.Rotation);
        }
        DrawLine(vertices[0].Position, vertices[1].Position, spriteRenderer.Tint);
        DrawLine(vertices[1].Position, vertices[2].Position, spriteRenderer.Tint);
        DrawLine(vertices[2].Position, vertices[3].Position, spriteRenderer.Tint);
        DrawLine(vertices[3].Position, vertices[0].Position, spriteRenderer.Tint);
    }

    void Renderer2D::DrawOutlineCall(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                                     F32 depth)
    {
        auto& referenceQuad = s_BatchData.ReferenceQuad;
        BatchVertex vertices[4];
        // Create new quad.
        for (U32 i = 0; i < 4; i++)
        {
            BatchVertex& vertex = vertices[i];
            vertex.Position = transform * glm::vec3(glm::vec2(referenceQuad.Position[i]), 1.0f) + glm::vec3{
                0.0f, 0.0f, depth
            };
        }
        DrawLine(vertices[0].Position, vertices[1].Position, spriteRenderer.Tint);
        DrawLine(vertices[1].Position, vertices[2].Position, spriteRenderer.Tint);
        DrawLine(vertices[2].Position, vertices[3].Position, spriteRenderer.Tint);
        DrawLine(vertices[3].Position, vertices[0].Position, spriteRenderer.Tint);
    }

    F32 Renderer2D::GetTextureIndex(BatchData& batch, Texture* texture)
    {
        F32 textureIndex = -1.0f;
        if (texture != nullptr)
        {
            bool textureFound = false;
            for (U32 i = 0; i < batch.CurrentTextureIndex; i++)
            {
                if (batch.UsedTextures[i] == texture)
                {
                    textureIndex = F32(i);
                    textureFound = true;
                    break;
                }
            }
            // If no such texture in the batch.
            if (textureFound == false)
            {
                if (batch.CurrentTextureIndex >= batch.MaxTextures)
                {
                    Flush(batch, *s_BatchData.TextShader);
                    ResetBatch(batch);
                }
                batch.UsedTextures[batch.CurrentTextureIndex] = texture;
                textureIndex = F32(batch.CurrentTextureIndex);
                batch.CurrentTextureIndex++;
            }
        }
        return textureIndex;
    }

    void Renderer2D::ShutDown()
    {
        s_BatchData.BatchShader.reset();
        s_BatchData.TextShader.reset();
        s_BatchData.LineShader.reset();
        s_BatchData.QuadBatch.VAO.reset();
        s_BatchData.PolygonBatch.VAO.reset();
        s_BatchData.TextBatch.VAO.reset();
        s_BatchData.LineBatch.VAO.reset();
        DeleteArr<U8>(s_BatchData.QuadBatch.VerticesMemory, s_BatchData.QuadBatch.MaxVertices * sizeof(BatchVertex));
        DeleteArr<U8>(s_BatchData.PolygonBatch.VerticesMemory,
                      s_BatchData.PolygonBatch.MaxVertices * sizeof(BatchVertex));
        DeleteArr<U8>(s_BatchData.PolygonBatch.IndicesMemory, s_BatchData.PolygonBatch.MaxIndices * sizeof(U32));
        DeleteArr<U8>(s_BatchData.TextBatch.VerticesMemory, s_BatchData.TextBatch.MaxVertices * sizeof(BatchVertex));
        DeleteArr<U8>(s_BatchData.LineBatch.VerticesMemory,
                      s_BatchData.LineBatch.MaxVertices * sizeof(BatchVertexLine));
        ENGINE_CORE_INFO("Renderer2D shutdown");
    }
}
