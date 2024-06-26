#include "enginepch.h"
#include "OpenGLRendererAPI.h"

#include "Engine/Core/Core.h"
#include "OpenGLBuffer.h"

#include "glad/glad.h"

namespace Engine
{
	void errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param);

	void OpenGLRendererAPI::Init()
	{
		ENGINE_CORE_INFO("Vendor: {}. Renderer: {}", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Set error callback
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(errorCallback, nullptr);
	}

	void OpenGLRendererAPI::SetViewport(U32 width, U32 height)
	{
		glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	}

	U32 OpenGLRendererAPI::GetNativeDataType(RendererAPI::DataType type)
	{
		switch (type)
		{
		case Engine::RendererAPI::DataType::UByte: return GL_UNSIGNED_BYTE;
		case Engine::RendererAPI::DataType::Byte:  return GL_BYTE;
		case Engine::RendererAPI::DataType::UInt:  return GL_UNSIGNED_INT;
		case Engine::RendererAPI::DataType::Int:   return GL_INT;
		case Engine::RendererAPI::DataType::Float: return GL_FLOAT;
		}
		ENGINE_CORE_FATAL("Unknown type");
		return 0;
	}

	void OpenGLRendererAPI::EnableCull(bool enable)
	{
		if (enable) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);
	}

	void OpenGLRendererAPI::SetCullFrontFace(CullFrontFace face)
	{
		switch (face)
		{
		case Engine::RendererAPI::CullFrontFace::Clockwise: glCullFace(GL_CW);
			break;
		case Engine::RendererAPI::CullFrontFace::CounterClockwise: glCullFace(GL_CCW);
			break;
		}
	}

	void OpenGLRendererAPI::ClearScreen()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec3& color)
	{
		m_ClearColor = color;
		glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, 1.0f);
	}

	void OpenGLRendererAPI::DrawIndexed(Ref<VertexArray> vertexArray)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, (GLsizei)vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawIndexed(Ref<VertexArray> vertexArray, U32 count)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, (GLsizei)count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawIndexed(Ref<VertexArray> vertexArray, PrimitiveType type)
	{
		DrawIndexed(vertexArray, vertexArray->GetIndexBuffer()->GetCount(), type);
	}

	void OpenGLRendererAPI::DrawIndexed(Ref<VertexArray> vertexArray, U32 count, PrimitiveType type)
	{
		vertexArray->Bind();
		switch (type)
		{
		case Engine::RendererAPI::PrimitiveType::Triangle:
			glDrawElements(GL_TRIANGLES, (GLsizei)count, GL_UNSIGNED_INT, nullptr);
			break;
		case Engine::RendererAPI::PrimitiveType::Line:
			glDrawElements(GL_LINES, (GLsizei)count, GL_UNSIGNED_INT, nullptr);
			break;
		}
	}


	void  OpenGLRendererAPI::SetDepthTestMode(Mode mode)
	{
		switch (mode)
		{
		case Engine::RendererAPI::Mode::Read: glDepthMask(false);
			break;
		case Engine::RendererAPI::Mode::ReadWrite: glDepthMask(true);
			break;
		}
	}

	void errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
	{
		auto sourceStr = [source]() -> std::string {
			switch (source)
			{
			case GL_DEBUG_SOURCE_API: return "API";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
			case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
			case GL_DEBUG_SOURCE_THIRD_PARTY:  return "THIRD PARTY";
			case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
			case GL_DEBUG_SOURCE_OTHER: return "OTHER";
			default: return "UNKNOWN";
			}
		}();

		auto typeStr = [type]() {
			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR: return "ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
			case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER:  return "MARKER";
			case GL_DEBUG_TYPE_OTHER: return "OTHER";
			default: return "UNKNOWN";
			}
		}();

		auto severityStr = [severity]() {
			switch (severity) {
			case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
			case GL_DEBUG_SEVERITY_LOW: return "LOW";
			case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
			case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
			default: return "UNKNOWN";
			}
		}();
		ENGINE_CORE_ERROR("OpenGL: {}, {}, {}, {}: {}", sourceStr, typeStr, severityStr, id, message);
		ENGINE_ASSERT(false, "OpenGL error")
	}
}



