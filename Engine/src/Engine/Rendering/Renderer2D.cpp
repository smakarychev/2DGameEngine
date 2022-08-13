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
		
		s_BatchData.ReferenceQuad.Position[0] = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
		s_BatchData.ReferenceQuad.Position[1] = glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f);
		s_BatchData.ReferenceQuad.Position[2] = glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f);
		s_BatchData.ReferenceQuad.Position[3] = glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f);
		
		s_BatchData.ReferenceQuad.UV[0] = glm::vec2(0.0f, 0.0f);
		s_BatchData.ReferenceQuad.UV[1] = glm::vec2(1.0f, 0.0f);
		s_BatchData.ReferenceQuad.UV[2] = glm::vec2(1.0f, 1.0f);
		s_BatchData.ReferenceQuad.UV[3] = glm::vec2(0.0f, 1.0f);
		
		s_BatchData.ReferenceQuad.Color[0] = glm::vec4(1.0f);
		s_BatchData.ReferenceQuad.Color[1] = glm::vec4(1.0f);
		s_BatchData.ReferenceQuad.Color[2] = glm::vec4(1.0f);
		s_BatchData.ReferenceQuad.Color[3] = glm::vec4(1.0f);

		// Indices structure is predictable.
		
		U32* indices = NewArr<U32>(s_BatchData.MaxVertices);
		for (U32 i = 0; i < s_BatchData.MaxVertices; i += 6)
		{
			U32 currVer = s_BatchData.CurrentVertices;
			indices[i] = currVer + 0;
			indices[i + 1] = currVer + 1;
			indices[i + 2] = currVer + 3;
			indices[i + 3] = currVer + 1;
			indices[i + 4] = currVer + 3;
			indices[i + 5] = currVer + 2;
			s_BatchData.CurrentVertices += 4;
		}
		
		auto vbo = VertexBuffer::Create(nullptr, s_BatchData.MaxVertices * sizeof(Vertex1P1UV1C));
		vbo->SetVertexLayout(Vertex1P1UV1C::GetLayout());
		auto ibo = IndexBuffer::Create(indices, s_BatchData.MaxVertices);
		DeleteArr<U32>(indices, s_BatchData.MaxVertices);

		s_BatchData.BatchVAO = VertexArray::Create();
		s_BatchData.BatchVAO->AddVertexBuffer(vbo);
		s_BatchData.BatchVAO->SetIndexBuffer(ibo);

		s_BatchData.QuadsVerticesMemory = reinterpret_cast<U8*>(MemoryManager::Alloc(s_BatchData.MaxVertices * sizeof(Vertex1P1UV1C)));
		s_BatchData.CurrentQuadVertexPointer = reinterpret_cast<Vertex1P1UV1C*>(s_BatchData.QuadsVerticesMemory);
	}

	void Renderer2D::BeginScene(std::shared_ptr<Camera> camera)
	{
		s_BatchData.CameraViewProjection = camera->GetViewProjection();
	}
	
	void Renderer2D::EndScene()
	{
		Flush();
		ResetBatch();
		//ENGINE_INFO("Renderer2D total draw calls: {}", s_BatchData.DrawCalls);
		s_BatchData.DrawCalls = 0;

	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& color)
	{
		if (s_BatchData.CurrentVertices + 4 > s_BatchData.MaxVertices)
		{
			Flush();
			ResetBatch();
		}

		auto& referenceQuad = s_BatchData.ReferenceQuad;

		// Create new quad.
		Vertex1P1UV1C vertices[4];
		for (int i = 0; i < 4; i++)
		{
			vertices[i].Position = glm::vec3(
				referenceQuad.Position[i].x * dimensions.x + position.x,
				referenceQuad.Position[i].y * dimensions.y + position.y,
				position.z
			);
			vertices[i].UV = referenceQuad.UV[i];
			vertices[i].Color = color;

			// Augment internal array.
			*s_BatchData.CurrentQuadVertexPointer = vertices[i];
			s_BatchData.CurrentQuadVertexPointer++;
		}

		// Add quad to buffer.
		ENGINE_CORE_ASSERT(s_BatchData.BatchVAO->GetVertexBuffers().size() == 1, "Batch must have only one vbo.");
		s_BatchData.CurrentQuads++;
		s_BatchData.CurrentVertices += 6;
	}

	void Renderer2D::Flush()
	{
		s_BatchData.BatchVAO->GetVertexBuffers()[0]->SetData(s_BatchData.QuadsVerticesMemory, sizeof(Vertex1P1UV1C) * s_BatchData.CurrentVertices);
		s_BatchData.BatchShader->Bind();
		s_BatchData.BatchShader->SetUniformMat4("u_modelViewProjection", s_BatchData.CameraViewProjection);
		s_BatchData.DrawCalls++;
		RenderCommand::DrawIndexed(s_BatchData.BatchVAO, s_BatchData.CurrentVertices);
	}

	void Renderer2D::ResetBatch()
	{
		s_BatchData.CurrentQuads = 0;
		s_BatchData.CurrentVertices = 0;
		s_BatchData.CurrentQuadVertexPointer = reinterpret_cast<Vertex1P1UV1C*>(s_BatchData.QuadsVerticesMemory);
	}

	void Renderer2D::ShutDown()
	{
		s_BatchData.BatchShader.~shared_ptr();
		s_BatchData.BatchVAO.~shared_ptr();
	}
}