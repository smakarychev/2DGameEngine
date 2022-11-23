#include "enginepch.h"

#include "Renderer2D.h"
#include "RenderCommand.h"

#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
    Renderer2D::BatchRendererData Renderer2D::s_BatchData;

    void Renderer2D::Init()
    {
        s_BatchData.TriangleBatchEditor.InitData("assets/shaders/batchShaderEditor.glsl");
        s_BatchData.TriangleBatch.InitData("assets/shaders/batchShader.glsl");

        s_BatchData.LineBatch.InitData("assets/shaders/lineShader.glsl");
        s_BatchData.LineBatch.SetPrimitiveType(RendererAPI::PrimitiveType::Line);
        
        s_BatchData.TextBatch.InitData("assets/shaders/textShader.glsl");

        
        s_BatchData.ReferenceQuad.Position = {
            glm::vec2{-0.5f, -0.5f}, glm::vec2{0.5f, -0.5f}, glm::vec2{0.5f, 0.5f}, glm::vec2{-0.5f, 0.5f}
        };
        s_BatchData.ReferenceQuad.Indices = {0, 1, 3, 1, 2, 3};
    }

    void Renderer2D::BeginScene(Camera* camera, SortingLayer* sortingLayer)
    {
        s_BatchData.CameraViewProjection = camera->GetViewProjection();
        s_BatchData.SortingLayer = sortingLayer;
        s_BatchData.Camera = camera;
        s_BatchData.RenderQueue.Clear();
        
        s_BatchData.TriangleBatch.SetCamera(camera);
        s_BatchData.LineBatch.SetCamera(camera);
        s_BatchData.TextBatch.SetCamera(camera);
        s_BatchData.TriangleBatchEditor.SetCamera(camera);
        s_BatchData.LineBatchEditor.SetCamera(camera);
        s_BatchData.TextBatchEditor.SetCamera(camera);
    }

    void Renderer2D::EndScene()
    {
        RenderCommand::EnableCull(false);
        if (s_BatchData.TriangleBatch.ShouldFlush())
        {
            s_BatchData.TriangleBatch.Flush();
            s_BatchData.TriangleBatch.Reset();
        }

        if (s_BatchData.LineBatch.ShouldFlush())
        {
            s_BatchData.LineBatch.Flush();
            s_BatchData.LineBatch.Reset();
        }

        if (s_BatchData.TextBatch.ShouldFlush())
        {
            RenderCommand::SetDepthTestMode(RendererAPI::Mode::Read);
            s_BatchData.TextBatch.Flush();
            s_BatchData.TextBatch.Reset();
            RenderCommand::SetDepthTestMode(RendererAPI::Mode::ReadWrite);
        }
        

        if (s_BatchData.TriangleBatchEditor.ShouldFlush())
        {
            s_BatchData.TriangleBatchEditor.Flush();
            s_BatchData.TriangleBatchEditor.Reset();
        }
        
        if (s_BatchData.LineBatchEditor.ShouldFlush())
        {
            s_BatchData.LineBatchEditor.Flush();
            s_BatchData.LineBatchEditor.Reset();
        }

        if (s_BatchData.TextBatchEditor.ShouldFlush())
        {
            RenderCommand::SetDepthTestMode(RendererAPI::Mode::Read);
            s_BatchData.TextBatchEditor.Flush();
            s_BatchData.TextBatchEditor.Reset();
            RenderCommand::SetDepthTestMode(RendererAPI::Mode::ReadWrite);
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

    void Renderer2D::DrawQuadEditor(U32 entityId, const Component::Transform2D& transform,
                                    const Component::SpriteRenderer& spriteRenderer,
                                    RendererAPI::PrimitiveType primitiveType)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(spriteRenderer.SortingLayer,
                                                                        spriteRenderer.OrderInLayer);
        DrawQuadEditorCall(entityId, transform, spriteRenderer, depth, primitiveType);
    }

    void Renderer2D::DrawQuadEditor(U32 entityId, const glm::mat3& transform,
                                    const Component::SpriteRenderer& spriteRenderer,
                                    RendererAPI::PrimitiveType primitiveType)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(spriteRenderer.SortingLayer,
                                                                        spriteRenderer.OrderInLayer);
        DrawQuadEditorCall(entityId, transform, spriteRenderer, depth, primitiveType);
    }

    void Renderer2D::DrawPolygonEditor(U32 entityId, const Component::Transform2D& transform,
                                       const Component::PolygonRenderer& polygonRenderer)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(polygonRenderer.SortingLayer,
                                                                        polygonRenderer.OrderInLayer);
        DrawPolygonEditorCall(entityId, transform, polygonRenderer, depth);
    }

    void Renderer2D::DrawPolygonEditor(U32 entityId, const glm::mat3& transform,
                                       const Component::PolygonRenderer& polygonRenderer)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(polygonRenderer.SortingLayer,
                                                                        polygonRenderer.OrderInLayer);
        DrawPolygonEditorCall(entityId, transform, polygonRenderer, depth);
    }

    void Renderer2D::DrawFontEditor(U32 entityId, const Component::FontRenderer& fontRenderer, const std::string& text)
    {
        const F32 depth = s_BatchData.SortingLayer->CalculateLayerDepth(fontRenderer.SortingLayer,
                                                                        fontRenderer.OrderInLayer);
        DrawFontEditorCall(entityId, fontRenderer, text, depth);
    }

    void Renderer2D::DrawFontFixedEditor(U32 entityId, const Component::FontRenderer& fontRenderer,
                                         const std::string& text)
    {
        DrawFontFixedEditorCall(entityId, fontRenderer, text);
    }

    void Renderer2D::DrawLineEditor(U32 entityId, const glm::vec2& from, const glm::vec2& to, const glm::vec4& color)
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
        s_BatchData.TriangleBatch.PushVertices(transform, depth, ShadingInfoCr::Create(spriteRenderer),
                                               s_BatchData.ReferenceQuad.Position, s_BatchData.ReferenceQuad.Indices);
    }

    void Renderer2D::DrawQuadCall(const glm::mat3& transform, const Component::SpriteRenderer& spriteRenderer,
                                  F32 depth, RendererAPI::PrimitiveType primitiveType)
    {
        if (primitiveType == RendererAPI::PrimitiveType::Line)
        {
            DrawOutlineCall(transform, spriteRenderer, depth);
            return;
        }
        s_BatchData.TriangleBatch.PushVertices(transform, depth, ShadingInfoCr::Create(spriteRenderer),
                                               s_BatchData.ReferenceQuad.Position, s_BatchData.ReferenceQuad.Indices);
    }

    void Renderer2D::DrawPolygonCall(const Component::Transform2D& transform,
                                     const Component::PolygonRenderer& polygonRenderer, F32 depth)
    {
        s_BatchData.TriangleBatch.PushVertices(transform, depth, ShadingInfoCr::Create(polygonRenderer),
                                               polygonRenderer.Polygon->GetVertices(),
                                               polygonRenderer.Polygon->GetIndices());
    }

    void Renderer2D::DrawPolygonCall(const glm::mat3& transform, const Component::PolygonRenderer& polygonRenderer,
                                     F32 depth)
    {
        s_BatchData.TriangleBatch.PushVertices(transform, depth, ShadingInfoCr::Create(polygonRenderer),
                                               polygonRenderer.Polygon->GetVertices(),
                                               polygonRenderer.Polygon->GetIndices());
    }

    void Renderer2D::DrawFontCall(const Component::FontRenderer& fontRenderer, const std::string& text, F32 depth)
    {
        const Font& font = *fontRenderer.Font;
        const F32 fontSizeCoeff = fontRenderer.FontSize / font.GetBaseFontSize() *
            s_BatchData.Camera->GetPixelCoefficient(1.0f);
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
            Component::Transform2D transform{
                glm::vec2{x, y} + font.GetCharacters()[ch].Bearing * fontSizeCoeff,
                glm::vec2{font.GetCharacters()[ch].Size * fontSizeCoeff},
                glm::vec2{1.0f, 0.0f}
            };
            s_BatchData.TextBatch.PushVertices(transform, depth, ShadingInfoCr::Create(fontRenderer, ch),
                                               s_BatchData.ReferenceQuad.Position, s_BatchData.ReferenceQuad.Indices);
            x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
        }
    }

    void Renderer2D::DrawFontFixedCall(const Component::FontRenderer& fontRenderer, const std::string& text)
    {
        const Font& font = *fontRenderer.Font;

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
            Component::Transform2D transform{
                glm::vec2{x, y} + font.GetCharacters()[ch].Bearing * fontSizeCoeff + glm::vec2{
                    s_BatchData.Camera->GetPosition()
                },
                glm::vec2{font.GetCharacters()[ch].Size * fontSizeCoeff},
                glm::vec2{1.0f, 0.0f}
            };
            s_BatchData.TextBatch.PushVertices(transform, 1.0f, ShadingInfoCr::Create(fontRenderer, ch),
                                               s_BatchData.ReferenceQuad.Position, s_BatchData.ReferenceQuad.Indices);
            x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
        }
    }

    void Renderer2D::DrawLineCall(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color, F32 depth)
    {
        ShadingInfo<std::array<glm::vec2, 2>> shi;
        shi.Tint = color;
        s_BatchData.LineBatch.PushVertices(depth, shi, std::array<glm::vec2, 2>{from, to}, std::array<U32, 2>{0, 1});
    }

    void Renderer2D::DrawQuadEditorCall(U32 entityId, const Component::Transform2D& transform,
                                        const Component::SpriteRenderer& spriteRenderer, F32 depth,
                                        RendererAPI::PrimitiveType primitiveType)
    {
        if (primitiveType == RendererAPI::PrimitiveType::Line)
        {
            DrawOutlineCall(transform, spriteRenderer, depth);
            return;
        }
        s_BatchData.TriangleBatchEditor.PushVertices(entityId, transform, depth, ShadingInfoCr::Create(spriteRenderer),
                                               s_BatchData.ReferenceQuad.Position, s_BatchData.ReferenceQuad.Indices);
    }

    void Renderer2D::DrawQuadEditorCall(U32 entityId, const glm::mat3& transform,
                                        const Component::SpriteRenderer& spriteRenderer, F32 depth,
                                        RendererAPI::PrimitiveType primitiveType)
    {
        if (primitiveType == RendererAPI::PrimitiveType::Line)
        {
            DrawOutlineCall(transform, spriteRenderer, depth);
            return;
        }
        s_BatchData.TriangleBatchEditor.PushVertices(entityId, transform, depth, ShadingInfoCr::Create(spriteRenderer),
                                               s_BatchData.ReferenceQuad.Position, s_BatchData.ReferenceQuad.Indices);
    }

    void Renderer2D::DrawPolygonEditorCall(U32 entityId, const Component::Transform2D& transform,
                                           const Component::PolygonRenderer& polygonRenderer, F32 depth)
    {
        s_BatchData.TriangleBatchEditor.PushVertices(entityId, transform, depth, ShadingInfoCr::Create(polygonRenderer),
                                               polygonRenderer.Polygon->GetVertices(),
                                               polygonRenderer.Polygon->GetIndices());
    }

    void Renderer2D::DrawPolygonEditorCall(U32 entityId, const glm::mat3& transform,
                                           const Component::PolygonRenderer& polygonRenderer, F32 depth)
    {
        s_BatchData.TriangleBatchEditor.PushVertices(entityId, transform, depth, ShadingInfoCr::Create(polygonRenderer),
                                               polygonRenderer.Polygon->GetVertices(),
                                               polygonRenderer.Polygon->GetIndices());
    }

    void Renderer2D::DrawFontEditorCall(U32 entityId, const Component::FontRenderer& fontRenderer,
                                        const std::string& text, F32 depth)
    {
        const Font& font = *fontRenderer.Font;
        const F32 fontSizeCoeff = fontRenderer.FontSize / font.GetBaseFontSize() *
            s_BatchData.Camera->GetPixelCoefficient(1.0f);
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
            Component::Transform2D transform{
                glm::vec2{x, y} + font.GetCharacters()[ch].Bearing * fontSizeCoeff,
                glm::vec2{font.GetCharacters()[ch].Size * fontSizeCoeff},
                glm::vec2{1.0f, 0.0f}
            };
            s_BatchData.TextBatchEditor.PushVertices(entityId, transform, depth, ShadingInfoCr::Create(fontRenderer, ch),
                                               s_BatchData.ReferenceQuad.Position, s_BatchData.ReferenceQuad.Indices);
            x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
        }
    }

    void Renderer2D::DrawFontFixedEditorCall(U32 entityId, const Component::FontRenderer& fontRenderer,
                                             const std::string& text)
    {
        const Font& font = *fontRenderer.Font;

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
            Component::Transform2D transform{
                glm::vec2{x, y} + font.GetCharacters()[ch].Bearing * fontSizeCoeff + glm::vec2{
                    s_BatchData.Camera->GetPosition()
                },
                glm::vec2{font.GetCharacters()[ch].Size * fontSizeCoeff},
                glm::vec2{1.0f, 0.0f}
            };
            s_BatchData.TextBatchEditor.PushVertices(entityId, transform, 1.0f, ShadingInfoCr::Create(fontRenderer, ch),
                                               s_BatchData.ReferenceQuad.Position, s_BatchData.ReferenceQuad.Indices);
            x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
        }
    }

    void Renderer2D::DrawLineEditorCall(U32 entityId, const glm::vec2& from, const glm::vec2& to,
                                        const glm::vec4& color, F32 depth)
    {
        ShadingInfo<std::array<glm::vec2, 2>> shi;
        shi.Tint = color;
        s_BatchData.LineBatchEditor.PushVertices(entityId, depth, shi, std::array<glm::vec2, 2>{from, to}, std::array<U32, 2>{0, 1});
    }

    void Renderer2D::DrawOutlineCall(const Component::Transform2D& transform,
                                     const Component::SpriteRenderer& spriteRenderer, F32 depth)
    {
        const auto& referenceQuad = s_BatchData.ReferenceQuad;
        BatchVertex vertices[4];
        // Create new quad.
        for (U32 i = 0; i < 4; i++)
        {
            BatchVertex& vertex = vertices[i];
            vertex.Position = glm::vec3(referenceQuad.Position[i], 0.0f);
            vertex.Position *= glm::vec3{transform.Scale, 1.0f};
            vertex.Position = glm::vec3{
                transform.Rotation.RotationVec.x * vertex.Position.x - transform.Rotation.RotationVec.y * vertex.Position.y,
                transform.Rotation.RotationVec.y * vertex.Position.x + transform.Rotation.RotationVec.x * vertex.Position.y,
                0.0f
            };
            vertex.Position += glm::vec3{transform.Position, depth};
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
            vertex.Position = transform * glm::vec3(referenceQuad.Position[i], 1.0f) + glm::vec3{
                0.0f, 0.0f, depth
            };
        }
        DrawLine(vertices[0].Position, vertices[1].Position, spriteRenderer.Tint);
        DrawLine(vertices[1].Position, vertices[2].Position, spriteRenderer.Tint);
        DrawLine(vertices[2].Position, vertices[3].Position, spriteRenderer.Tint);
        DrawLine(vertices[3].Position, vertices[0].Position, spriteRenderer.Tint);
    }

    void Renderer2D::ShutDown()
    {
        s_BatchData.TriangleBatch.Shutdown();
        s_BatchData.LineBatch.Shutdown();
        s_BatchData.TextBatch.Shutdown();
        
        s_BatchData.TriangleBatchEditor.Shutdown();
        s_BatchData.LineBatchEditor.Shutdown();
        s_BatchData.TextBatchEditor.Shutdown();

        ENGINE_CORE_INFO("Renderer2D shutdown");
    }
}
