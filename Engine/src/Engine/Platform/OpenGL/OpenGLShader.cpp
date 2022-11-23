#include "enginepch.h"

#include "OpenGLShader.h"

namespace Engine
{
	OpenGLShader::OpenGLShader(std::string name, std::string source) : m_Id(0), m_Name(std::move(name))
	{
		auto shadersSources = ExtractShaders(source);
		CreateShaderProgram(shadersSources);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_Id);
	}

	void OpenGLShader::Bind()
	{
		glUseProgram(m_Id);
	}

	std::map<GLuint, std::string> OpenGLShader::ExtractShaders(const std::string& source)
	{
		constexpr static std::string_view prefixToken = "@shaderType";
		constexpr static U64 prefixSize = prefixToken.size();

		std::map<GLuint, std::string> shaderMap;

		U64 pos = source.find(prefixToken);
		while (pos != std::string::npos)
		{
			U64 shaderNameBegin = pos + prefixSize + 1;
			U64 shaderNameEnd = source.find_first_of("\r\n", pos);
			if (shaderNameBegin == std::string::npos || shaderNameEnd == source.size() - 1) ENGINE_FATAL("Invalid shader file");
			std::string shaderName = source.substr(shaderNameBegin, shaderNameEnd - shaderNameBegin);

			U64 shaderSourceBegin = source.find_first_not_of("\r\n", shaderNameEnd);
			pos = source.find(prefixToken, shaderSourceBegin);
			U64 shaderSourceEnd = (pos == std::string::npos) ? source.size() : pos;

			shaderMap.emplace(ShaderNameToOpenGLShaderType(shaderName), source.substr(shaderSourceBegin, shaderSourceEnd - shaderSourceBegin));
		}
		return shaderMap;
	}
	
	void OpenGLShader::CreateShaderProgram(const std::map<GLuint, std::string> sources)
	{
		m_Id = glCreateProgram();
		for (auto&& [shaderType, shaderSource] : sources)
		{
			U32 shaderId = glCreateShader(shaderType);
			const char* shaderSourceCString = shaderSource.c_str();
			glShaderSource(shaderId, 1, &shaderSourceCString, nullptr);
			glCompileShader(shaderId);

			// Check for shader compile errors.
			I32 success;
			char infoLog[512];
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shaderId, 512, nullptr, infoLog);
				ENGINE_ERROR("Shader compile error: {}", infoLog);
			}
			glAttachShader(m_Id, shaderId);
			glDeleteShader(shaderId);
		}
		glLinkProgram(m_Id);
		// Check for linking errors.
		I32 success;
		char infoLog[512];
		glGetProgramiv(m_Id, GL_LINK_STATUS, &success);
		if (!success) 
		{
			glGetProgramInfoLog(m_Id, 512, nullptr, infoLog);
			ENGINE_ERROR("Shader program link failed: {}", infoLog);
		}
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, void* value, ShaderUniformType type)
	{
		Bind();
		switch (type)
		{
		case Engine::ShaderUniformType::Float:	glUniform1fv(GetUniformLocation(uniformName), 1, (GLfloat*)value);
			break;
		case Engine::ShaderUniformType::Float2:	glUniform2fv(GetUniformLocation(uniformName), 1, (GLfloat*)value);
			break;
		case Engine::ShaderUniformType::Float3:	glUniform3fv(GetUniformLocation(uniformName), 1, (GLfloat*)value);
			break;
		case Engine::ShaderUniformType::Float4:	glUniform4fv(GetUniformLocation(uniformName), 1, (GLfloat*)value);
			break;
		case Engine::ShaderUniformType::Mat2:	glUniformMatrix2fv(GetUniformLocation(uniformName), 1, GL_FALSE, (GLfloat*)value);
			break;
		case Engine::ShaderUniformType::Mat3:	glUniformMatrix3fv(GetUniformLocation(uniformName), 1, GL_FALSE, (GLfloat*)value);
			break;
		case Engine::ShaderUniformType::Mat4:	glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, (GLfloat*)value);
			break;
		case Engine::ShaderUniformType::Int:	glUniform1iv(GetUniformLocation(uniformName),  1, (GLint*)value);
			break;
		case Engine::ShaderUniformType::UInt:	glUniform1uiv(GetUniformLocation(uniformName), 1, (GLuint*)value);
			break;
		case Engine::ShaderUniformType::Bool:	glUniform1iv(GetUniformLocation(uniformName),  1, (GLint*)value);
			break;
		}
	}

	void OpenGLShader::SetUniformInt(const std::string& uniformName, I32 value)
	{
		glUniform1iv(GetUniformLocation(uniformName), 1, &value);
	}

	void OpenGLShader::SetUniformUInt(const std::string& uniformName, U32 value)
	{
		glUniform1uiv(GetUniformLocation(uniformName), 1, &value);

	}

	void OpenGLShader::SetUniformFloat(const std::string& uniformName, F32 value)
	{
		glUniform1fv(GetUniformLocation(uniformName), 1, &value);
	}

	void OpenGLShader::SetUniformFloat2(const std::string& uniformName, const glm::vec2& value)
	{
		glUniform2fv(GetUniformLocation(uniformName), 1, &value[0]);
	}

	void OpenGLShader::SetUniformFloat3(const std::string& uniformName, const glm::vec3& value)
	{
		glUniform3fv(GetUniformLocation(uniformName), 1, &value[0]);
	}

	void OpenGLShader::SetUniformMat2(const std::string& uniformName, const glm::mat2& value)
	{
		glUniformMatrix2fv(GetUniformLocation(uniformName), 1, GL_FALSE, &value[0][0]);
	}

	void OpenGLShader::SetUniformMat3(const std::string& uniformName, const glm::mat3& value)
	{
		glUniformMatrix3fv(GetUniformLocation(uniformName), 1, GL_FALSE, &value[0][0]);
	}

	void OpenGLShader::SetUniformMat4(const std::string& uniformName, const glm::mat4& value)
	{
		glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, &value[0][0]);
	}

	void OpenGLShader::SetUniformBool(const std::string& uniformName, bool value)
	{
		glUniform1iv(GetUniformLocation(uniformName), 1, (GLint*)&value);
	}

	GLint OpenGLShader::GetUniformLocation(const std::string& uniformName)
	{
		auto it = m_UniformLocations.find(uniformName);
		if (it != m_UniformLocations.end()) return it->second;
		
		GLint uniformLocation = glGetUniformLocation(m_Id, uniformName.c_str());
		if (uniformLocation == -1) ENGINE_CORE_ERROR("Unknown uniform name: {}", uniformName);
		m_UniformLocations.emplace(uniformName, uniformLocation);
		return uniformLocation;
	}

}