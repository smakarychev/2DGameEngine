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
			indices[i + 4]	= currVer + 3;
			indices[i + 5]	= currVer + 2;
			quadBatch.CurrentVertices += 4;
		}
		quadBatch.CurrentVertices = 0;

		auto vbo = VertexBuffer::Create(nullptr, quadBatch.MaxVertices * sizeof(BatchVertex));
		vbo->SetVertexLayout(BatchVertex::GetLayout());
		auto ibo = IndexBuffer::Create(indices, quadBatch.MaxIndices);
		DeleteArr<U32>(indices, quadBatch.MaxIndices);
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
	}

	void Renderer2D::BeginScene(std::shared_ptr<Camera> camera)
	{
		s_BatchData.CameraViewProjection = camera->GetViewProjection();
	}
	
	void Renderer2D::EndScene()
	{
		Flush(s_BatchData.QuadBatch);
		ResetBatch(s_BatchData.QuadBatch);
		Flush(s_BatchData.PolygonBatch);
		ResetBatch(s_BatchData.PolygonBatch);
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

	void Renderer2D::Flush(BatchData& batch)
	{
		s_BatchData.BatchShader->Bind();

		for (U32 i = 0; i < batch.CurrentTextureIndex; i++)
		{
			batch.UsedTextures[i]->Bind(i);
		}

		ENGINE_CORE_ASSERT(batch.VAO->GetVertexBuffers().size() == 1, "Batch must have only one vbo.");
		
		batch.VAO->GetVertexBuffers()[0]->SetData(batch.VerticesMemory, sizeof(BatchVertex) * batch.CurrentVertices);
		if (&batch == &s_BatchData.PolygonBatch) 
			batch.VAO->GetIndexBuffer()->SetData(reinterpret_cast<U32*>(batch.IndicesMemory), batch.CurrentIndices);

		s_BatchData.BatchShader->SetUniformMat4("u_modelViewProjection", s_BatchData.CameraViewProjection);
		s_BatchData.DrawCalls++;
		RenderCommand::DrawIndexed(batch.VAO, batch.CurrentIndices);
	}

	void Renderer2D::ResetBatch(BatchData& batch)
	{
		batch.CurrentVertices = 0;
		batch.CurrentIndices = 0;
		batch.CurrentTextureIndex = 0;
		batch.CurrentVertexPointer = reinterpret_cast<BatchVertex*>(batch.VerticesMemory);
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
					Flush(batch);
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
		s_BatchData.QuadBatch.VAO.~shared_ptr();
		s_BatchData.PolygonBatch.VAO.~shared_ptr();
		DeleteArr<U8>(s_BatchData.QuadBatch.VerticesMemory, s_BatchData.QuadBatch.MaxVertices * sizeof(BatchVertex));
		DeleteArr<U8>(s_BatchData.PolygonBatch.VerticesMemory, s_BatchData.PolygonBatch.MaxVertices * sizeof(BatchVertex));
		DeleteArr<U8>(s_BatchData.PolygonBatch.IndicesMemory, s_BatchData.PolygonBatch.MaxIndices * sizeof(U32));
	}
}