#include "enginepch.h"

#include "Renderer2D.h"

#include "Engine/Core/Core.h"

#include "RenderCommand.h"

namespace Engine
{
	Renderer2D::BatchRendererData Renderer2D::s_BatchData;

	void Renderer2D::Init()
	{
		// Initialize batch shader and batch buffers.
		s_BatchData.BatchShader = Shader::ReadShaderFromFile("assets/shaders/batchShader.glsl");
		s_BatchData.TextShader = Shader::ReadShaderFromFile("assets/shaders/textShader.glsl");
		s_BatchData.LineShader= Shader::ReadShaderFromFile("assets/shaders/lineShader.glsl");
		
		//*************** Init quad **********************************************************
		s_BatchData.ReferenceQuad.Position.resize(4);
		s_BatchData.ReferenceQuad.Position[0] = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
		s_BatchData.ReferenceQuad.Position[1] = glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f);
		s_BatchData.ReferenceQuad.Position[2] = glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f);
		s_BatchData.ReferenceQuad.Position[3] = glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f);
		
		s_BatchData.ReferenceQuad.UV.resize(4);
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
			indices[i]		= currVer + 0;
			indices[i + 1]	= currVer + 1;
			indices[i + 2]	= currVer + 3;
			indices[i + 3]	= currVer + 1;
			indices[i + 4]	= currVer + 2;
			indices[i + 5]	= currVer + 3;
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

		//*************** Init regular polygon ***********************************************
		BatchData polygonBatch;
		vbo = VertexBuffer::Create(nullptr, polygonBatch.MaxVertices * sizeof(BatchVertex));
		vbo->SetVertexLayout(BatchVertex::GetLayout());
		ibo = IndexBuffer::Create(indices, polygonBatch.MaxIndices);
		polygonBatch.VAO = VertexArray::Create();
		polygonBatch.VAO->AddVertexBuffer(vbo);
		polygonBatch.VAO->SetIndexBuffer(ibo);

		polygonBatch.VerticesMemory = NewArr<U8>(polygonBatch.MaxVertices * sizeof(BatchVertex));
		polygonBatch.CurrentVertexPointer = reinterpret_cast<BatchVertex*>(polygonBatch.VerticesMemory);
		polygonBatch.IndicesMemory = NewArr<U8>(polygonBatch.MaxIndices * sizeof(U32));
		polygonBatch.CurrentIndexPointer = reinterpret_cast<U32*>(polygonBatch.IndicesMemory);
		s_BatchData.PolygonBatch = polygonBatch;

		//*************** Init text **********************************************************
		BatchData textBatch;
		vbo = VertexBuffer::Create(nullptr, textBatch.MaxVertices * sizeof(BatchVertex));
		vbo->SetVertexLayout(BatchVertex::GetLayout());
		ibo = IndexBuffer::Create(indices, textBatch.MaxIndices);
		textBatch.VAO = VertexArray::Create();
		textBatch.VAO->AddVertexBuffer(vbo);
		textBatch.VAO->SetIndexBuffer(ibo);

		textBatch.VerticesMemory = NewArr<U8>(textBatch.MaxVertices * sizeof(BatchVertex));
		textBatch.CurrentVertexPointer = reinterpret_cast<BatchVertex*>(textBatch.VerticesMemory);
		s_BatchData.TextBatch = textBatch;

		DeleteArr<U32>(indices, quadBatch.MaxIndices);

		//*************** Init lines *********************************************************
		BatchDataLines lineBatch;
		vbo = VertexBuffer::Create(nullptr, lineBatch.MaxVertices * sizeof(BatchVertexLine));
		vbo->SetVertexLayout(BatchVertexLine::GetLayout());
		indices = NewArr<U32>(lineBatch.MaxIndices);
		for (U32 i = 0; i + 2 < lineBatch.MaxIndices; i += 2)
		{
			indices[i]		= i;
			indices[i + 1]	= i + 1;
		}
		ibo = IndexBuffer::Create(indices, lineBatch.MaxIndices);
		lineBatch.VAO = VertexArray::Create();
		lineBatch.VAO->AddVertexBuffer(vbo);
		lineBatch.VAO->SetIndexBuffer(ibo);

		lineBatch.VerticesMemory = NewArr<U8>(lineBatch.MaxVertices * sizeof(BatchVertexLine));
		lineBatch.CurrentVertexPointer = reinterpret_cast<BatchVertexLine*>(lineBatch.VerticesMemory);
		s_BatchData.LineBatch = lineBatch;

		DeleteArr<U32>(indices, lineBatch.MaxIndices);
	}

	void Renderer2D::BeginScene(Ref<Camera> camera)
	{
		s_BatchData.CameraViewProjection = camera->GetViewProjection();
		s_BatchData.Camera = camera.get();
	}
	
	void Renderer2D::EndScene()
	{
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

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color)
	{
		DrawQuad(position, scale, nullptr, s_BatchData.ReferenceQuad.UV, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, F32 rotation, const glm::vec4& color)
	{
		DrawQuad(position, scale, rotation, nullptr, {}, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, Texture& texture, const glm::vec2& textureTiling)
	{
		DrawQuad(position, scale, &texture, s_BatchData.ReferenceQuad.UV, glm::vec4(1.0f), textureTiling);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, F32 rotation, Texture& texture, const glm::vec2& textureTiling)
	{
		DrawQuad(position, scale, rotation, &texture, s_BatchData.ReferenceQuad.UV, glm::vec4(1.0f), textureTiling);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, Texture& texture, const glm::vec4& tint, const glm::vec2& textureTiling)
	{
		DrawQuad(position, scale, &texture, s_BatchData.ReferenceQuad.UV, tint, textureTiling);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, F32 rotation, Texture& texture, const glm::vec4& tint, const glm::vec2& textureTiling)
	{
		DrawQuad(position, scale, rotation, &texture, {}, tint, textureTiling);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, Texture& texture, const std::vector<glm::vec2>& uv, const glm::vec4& tint, const glm::vec2& textureTiling)
	{
		DrawQuad(position, scale, &texture, uv, tint, textureTiling);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, Texture* texture, const std::vector<glm::vec2>& uv, const glm::vec4& tint, const glm::vec2& textureTiling)
	{
		DrawQuad(position, scale, 0.0f, texture, uv, tint, textureTiling);
	}

	void Renderer2D::DrawQuad(
		const glm::vec3& position, const glm::vec2& scale, F32 rotation,
		Texture* texture, const std::vector<glm::vec2>& uv,
		const glm::vec4& tint, const glm::vec2& textureTiling
	)
	{
		BatchData& quadBatch = s_BatchData.QuadBatch;
		if (quadBatch.CurrentVertices + 4 > quadBatch.MaxVertices || quadBatch.CurrentIndices + 6 > quadBatch.MaxIndices)
		{
			Flush(quadBatch);
			ResetBatch(quadBatch);
		}
		F32 textureIndex = GetTextureIndex(quadBatch, texture);
		auto& referenceQuad = s_BatchData.ReferenceQuad;

		// Create new quad.
		for (U32 i = 0; i < 4; i++)
		{
			BatchVertex* vertex = quadBatch.CurrentVertexPointer;
			vertex->Position = glm::vec3(referenceQuad.Position[i]);
			if (rotation == 0.0f) InitVertexGeometryData(*vertex, position, scale);
			else InitVertexGeometryData(*vertex, position, scale, rotation);
			InitVertexColorData(*vertex, textureIndex, uv[i], tint, textureTiling);

			quadBatch.CurrentVertexPointer++;
		}
		quadBatch.CurrentVertices += 4;
		quadBatch.CurrentIndices += 6;
	}

	void Renderer2D::DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color)
	{
		DrawPolygon(polygon, position, scale, nullptr, color);
	}

	void Renderer2D::DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, F32 rotation, const glm::vec4& color)
	{
		DrawPolygon(polygon, position, scale, rotation, nullptr, color);
	}

	void Renderer2D::DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, Texture& texture, const glm::vec2& textureTiling)
	{
		DrawPolygon(polygon, position, scale, &texture, glm::vec4(1.0f), textureTiling);
	}

	void Renderer2D::DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, F32 rotation, Texture& texture, const glm::vec2& textureTiling)
	{
		DrawPolygon(polygon, position, scale, rotation, &texture, glm::vec4(1.0f), textureTiling);
	}

	void Renderer2D::DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, Texture& texture, const glm::vec4& tint, const glm::vec2& textureTiling)
	{
		DrawPolygon(polygon, position, scale, &texture, tint, textureTiling);
	}

	void Renderer2D::DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, Texture* texture, const glm::vec4& tint, const glm::vec2& textureTiling)
	{
		DrawPolygon(polygon, position, scale, 0.0f, texture, tint, textureTiling);
	}

	void Renderer2D::DrawPolygon(const RegularPolygon& polygon, const glm::vec3& position, const glm::vec2& scale, F32 rotation, Texture* texture, const glm::vec4& tint, const glm::vec2& textureTiling)
	{
		BatchData& polygonBatch = s_BatchData.PolygonBatch;
		if (polygonBatch.CurrentVertices + polygon.GetVertices().size() > polygonBatch.MaxVertices)
		{
			Flush(s_BatchData.PolygonBatch);
			ResetBatch(s_BatchData.PolygonBatch);
		}
		F32 textureIndex = GetTextureIndex(polygonBatch, texture);

		for (U32 i = 0; i < polygon.GetNumberOfVertices(); i++)
		{
			BatchVertex* vertex = polygonBatch.CurrentVertexPointer;
			vertex->Position = glm::vec3(polygon.GetVertices()[i], 0.0f);
			if (rotation == 0.0f) InitVertexGeometryData(*vertex, position, scale);
			else InitVertexGeometryData(*vertex, position, scale, rotation);
			InitVertexColorData(*vertex, textureIndex, polygon.GetUVs()[i], tint, textureTiling);
			polygonBatch.CurrentVertexPointer++;
		}
		for (U32 i = 0; i < polygon.GetIndices().size(); i++)
		{
			U32* index = polygonBatch.CurrentIndexPointer;
			*index = polygon.GetIndices()[i] + polygonBatch.CurrentVertices;
			polygonBatch.CurrentIndexPointer++;
		}
		polygonBatch.CurrentVertices += U32(polygon.GetVertices().size());
		polygonBatch.CurrentIndices += U32(polygon.GetIndices().size());
	}

	void Renderer2D::DrawFontFixed(Font& font, F32 fontSize, const std::string& text, const glm::vec4& color)
	{
		DrawFontFixed(font, fontSize, 0.0f, std::numeric_limits<F32>::max(), 0.0f, text, color);
	}

	void Renderer2D::DrawFontFixed(Font& font, F32 fontSize, F32 xminPx, F32 xmaxPx, F32 yminPx, const std::string& text, const glm::vec4& color)
	{
		BatchData& textBatch = s_BatchData.TextBatch;
		if (textBatch.CurrentVertices + 4 > textBatch.MaxVertices || textBatch.CurrentIndices + 6 > textBatch.MaxIndices)
		{
			Flush(textBatch, *s_BatchData.TextShader);
			ResetBatch(textBatch);
		}
		F32 textureIndex = GetTextureIndex(textBatch, &font.GetAtlas());
		auto& referenceQuad = s_BatchData.ReferenceQuad;

		F32 fontSizeCoeff = fontSize / font.GetBaseFontSize() * s_BatchData.Camera->GetPixelCoefficient();
		xminPx -= (F32)s_BatchData.Camera->GetViewportWidth()  / 2.0f;
		xmaxPx -= (F32)s_BatchData.Camera->GetViewportWidth()  / 2.0f;
		yminPx =  (F32)s_BatchData.Camera->GetViewportHeight() / 2.0f - fontSize - yminPx;
		xminPx *= s_BatchData.Camera->GetPixelCoefficient(); xmaxPx *= s_BatchData.Camera->GetPixelCoefficient(); yminPx *= s_BatchData.Camera->GetPixelCoefficient();
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
				BatchVertex* vertex = textBatch.CurrentVertexPointer;
				vertex->Position = glm::vec3(referenceQuad.Position[i]);
				InitVertexGeometryData(
					*vertex,
					glm::vec3{ x, y, s_BatchData.Camera->GetPosition().z - s_BatchData.Camera->GetNearClipPlane() * 1.001f } +
						glm::vec3(font.GetCharacters()[ch].Bearing * fontSizeCoeff, 0.0f) +
						glm::vec3(glm::vec2(s_BatchData.Camera->GetPosition()), 0.0f),
					font.GetCharacters()[ch].Size * fontSizeCoeff);
				InitVertexColorData(*vertex, textureIndex, font.GetCharacters()[ch].UV[i], color, glm::vec2{ 1.0f });

				textBatch.CurrentVertexPointer++;
			}
			textBatch.CurrentVertices += 4;
			textBatch.CurrentIndices += 6;
			x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
		}
	}

	void Renderer2D::DrawFont(Font& font, F32 fontSize, F32 xmin, F32 xmax, F32 ymin, const std::string& text, const glm::vec4& color)
	{
		BatchData& textBatch = s_BatchData.TextBatch;
		if (textBatch.CurrentVertices + 4 > textBatch.MaxVertices || textBatch.CurrentIndices + 6 > textBatch.MaxIndices)
		{
			Flush(textBatch, *s_BatchData.TextShader);
			ResetBatch(textBatch);
		}
		F32 textureIndex = GetTextureIndex(textBatch, &font.GetAtlas());
		auto& referenceQuad = s_BatchData.ReferenceQuad;

		F32 fontSizeCoeff = fontSize / font.GetBaseFontSize() * s_BatchData.Camera->GetPixelCoefficient(1.0f);
		F32 x = xmin;
		F32 y = ymin;

		for (auto& ch : text)
		{
			if (ch == ' ')
			{
				x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
				continue;
			}

			if (x + font.GetCharacters()[ch].Size.x * fontSizeCoeff > xmax)
			{
				y -= font.GetLineHeight() * fontSizeCoeff;
				x = xmin;
			}
			// Create new quad.
			for (U32 i = 0; i < 4; i++)
			{
				BatchVertex* vertex = textBatch.CurrentVertexPointer;
				vertex->Position = glm::vec3(referenceQuad.Position[i]);
				InitVertexGeometryData(
					*vertex,
					glm::vec3{ x, y, 0.0f } +
					glm::vec3(font.GetCharacters()[ch].Bearing * fontSizeCoeff, 0.0f),
					font.GetCharacters()[ch].Size * fontSizeCoeff);
				InitVertexColorData(*vertex, textureIndex, font.GetCharacters()[ch].UV[i], color, glm::vec2{ 1.0f });

				textBatch.CurrentVertexPointer++;
			}
			textBatch.CurrentVertices += 4;
			textBatch.CurrentIndices += 6;
			x += font.GetCharacters()[ch].Advance * fontSizeCoeff;
		}
	}

	void Renderer2D::DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color)
	{
		DrawLine(glm::vec3{ from, 0.0f }, glm::vec3{ to, 0.0f }, color);
	}

	void Renderer2D::DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color)
	{
		BatchDataLines& lineBatch = s_BatchData.LineBatch;
		if (lineBatch.CurrentVertices + 2 > lineBatch.MaxVertices || lineBatch.CurrentIndices + 2 > lineBatch.MaxIndices)
		{
			Flush(lineBatch);
			ResetBatch(lineBatch);
		}

		// Create new line.
		BatchVertexLine* vertex = lineBatch.CurrentVertexPointer;
		vertex->Position = from;
		vertex->Color = color;
		lineBatch.CurrentVertexPointer++;
		vertex = lineBatch.CurrentVertexPointer;
		vertex->Position = to;
		vertex->Color = color;
		lineBatch.CurrentVertexPointer++;

		lineBatch.CurrentVertices += 2;
		lineBatch.CurrentIndices += 2;
	}

	void Renderer2D::Flush(BatchData& batch, Shader& shader)
	{
		shader.Bind();

		for (U32 i = 0; i < batch.CurrentTextureIndex; i++)
		{
			batch.UsedTextures[i]->Bind(i);
		}

		ENGINE_CORE_ASSERT(batch.VAO->GetVertexBuffers().size() == 1, "Batch must have only one vbo.");
		
		batch.VAO->GetVertexBuffers()[0]->SetData(batch.VerticesMemory, sizeof(BatchVertex) * batch.CurrentVertices);
		if (&batch == &s_BatchData.PolygonBatch) 
			batch.VAO->GetIndexBuffer()->SetData(reinterpret_cast<U32*>(batch.IndicesMemory), batch.CurrentIndices);
		if (&batch == &s_BatchData.TextBatch)
			RenderCommand::SetDepthTestMode(RendererAPI::Mode::Read);

		shader.SetUniformMat4("u_modelViewProjection", s_BatchData.CameraViewProjection);
		s_BatchData.DrawCalls++;
		RenderCommand::DrawIndexed(batch.VAO, batch.CurrentIndices);
		if (&batch == &s_BatchData.TextBatch)
			RenderCommand::SetDepthTestMode(RendererAPI::Mode::ReadWrite);
	}

	void Renderer2D::Flush(BatchDataLines& batch, Shader& shader)
	{
		shader.Bind();
		ENGINE_CORE_ASSERT(batch.VAO->GetVertexBuffers().size() == 1, "Batch must have only one vbo.");
		batch.VAO->GetVertexBuffers()[0]->SetData(batch.VerticesMemory, sizeof(BatchVertexLine) * batch.CurrentVertices);
		
		shader.SetUniformMat4("u_modelViewProjection", s_BatchData.CameraViewProjection);
		s_BatchData.DrawCalls++;
		RenderCommand::DrawIndexed(batch.VAO, batch.CurrentIndices, RendererAPI::PrimitiveType::Line);
	}

	void Renderer2D::ResetBatch(BatchData& batch)
	{
		batch.CurrentVertices = 0;
		batch.CurrentIndices = 0;
		batch.CurrentTextureIndex = 0;
		batch.CurrentVertexPointer = reinterpret_cast<BatchVertex*>(batch.VerticesMemory);
		batch.CurrentIndexPointer = reinterpret_cast<U32*>(batch.IndicesMemory);
	}

	void Renderer2D::ResetBatch(BatchDataLines& batch)
	{
		batch.CurrentVertices = 0;
		batch.CurrentIndices = 0;
		batch.CurrentVertexPointer = reinterpret_cast<BatchVertexLine*>(batch.VerticesMemory);
		batch.CurrentIndexPointer = reinterpret_cast<U32*>(batch.IndicesMemory);
	}

	void Renderer2D::InitVertexGeometryData(BatchVertex& vertex, const glm::vec3& position, const glm::vec2& scale)
	{
		vertex.Position = vertex.Position * glm::vec3(scale, 1.0) + position;
	}

	void Renderer2D::InitVertexGeometryData(BatchVertex& vertex, const glm::vec3& position, const glm::vec2& scale, F32 rotation)
	{
		vertex.Position = glm::vec3{
			glm::cos(rotation) * vertex.Position.x - glm::sin(rotation) * vertex.Position.y,
			glm::sin(rotation) * vertex.Position.x + glm::cos(rotation) * vertex.Position.y,
			vertex.Position.z
		};
		vertex.Position = vertex.Position * glm::vec3(scale, 1.0) + position;
	}

	void Renderer2D::InitVertexColorData(BatchVertex& vertex, F32 textureIndex, const glm::vec2& uv, const glm::vec4& tint, const glm::vec2& textureTiling)
	{
		vertex.TextureIndex = textureIndex;
		vertex.Color = tint;
		vertex.TextureTiling = textureTiling;
		vertex.UV = uv;
	}

	F32 Renderer2D::GetTextureIndex(BatchData& batch, Texture* texture)
	{
		F32 textureIndex = -1.0f;
		if (texture != nullptr)
		{
			for (U32 i = 0; i < batch.CurrentTextureIndex; i++)
			{
				if (batch.UsedTextures[i] == texture) textureIndex = F32(i);
			}
			// If no such texture in the batch.
			if (textureIndex == -1.0f)
			{
				if (batch.CurrentTextureIndex >= batch.MaxTextures)
				{
					Flush(batch, *s_BatchData.TextShader);
					ResetBatch(batch);
				}
				batch.UsedTextures[batch.CurrentTextureIndex] = const_cast<Texture*>(texture);
				textureIndex = F32(batch.CurrentTextureIndex);
				batch.CurrentTextureIndex++;
			}
		}
		return textureIndex;
	}


	void Renderer2D::ShutDown()
	{
		s_BatchData.BatchShader.~shared_ptr();
		s_BatchData.TextShader.~shared_ptr();
		s_BatchData.LineShader.~shared_ptr();
		s_BatchData.QuadBatch.VAO.~shared_ptr();
		s_BatchData.PolygonBatch.VAO.~shared_ptr();
		s_BatchData.TextBatch.VAO.~shared_ptr();
		s_BatchData.LineBatch.VAO.~shared_ptr();
		DeleteArr<U8>(s_BatchData.QuadBatch.VerticesMemory, s_BatchData.QuadBatch.MaxVertices * sizeof(BatchVertex));
		DeleteArr<U8>(s_BatchData.PolygonBatch.VerticesMemory, s_BatchData.PolygonBatch.MaxVertices * sizeof(BatchVertex));
		DeleteArr<U8>(s_BatchData.PolygonBatch.IndicesMemory, s_BatchData.PolygonBatch.MaxIndices * sizeof(U32));
		DeleteArr<U8>(s_BatchData.TextBatch.VerticesMemory, s_BatchData.TextBatch.MaxVertices * sizeof(BatchVertex));
		DeleteArr<U8>(s_BatchData.LineBatch.VerticesMemory, s_BatchData.LineBatch.MaxVertices * sizeof(BatchVertexLine));
	}
}