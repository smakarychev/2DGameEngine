#pragma once

#include "Engine/Core/Log.h"
#include "Engine/Core/Types.h"
#include "Engine/Rendering/Shader.h"

#include "glad/glad.h"

namespace Engine
{
	using namespace Types;

	inline GLuint ShaderNameToOpenGLShaderType(const std::string& name)
	{
		static std::unordered_map<std::string, GLuint> nameTypeMap
		{
			{"VERTEX",   GL_VERTEX_SHADER},
			{"PIXEL",    GL_FRAGMENT_SHADER},
			{"HULL",     GL_TESS_CONTROL_SHADER},
			{"DOMAIN",   GL_TESS_EVALUATION_SHADER},
			{"GEOMETRY", GL_GEOMETRY_SHADER},
		};
		auto it = nameTypeMap.find(name);
		if (it == nameTypeMap.end())
		{
			ENGINE_CORE_FATAL("Unrecognized shader type: {}", name);
			return GL_FALSE;
		}
		return it->second;
	}

	class OpenGLShader : public Shader
	{

	public:
		OpenGLShader(const std::string name, const std::string& source);

		U32 GetId() const override { return m_Id; };

		virtual void Bind() override;
		
		void SetUniform(				const std::string& uniformName, void* value, ShaderUniformType type) override;
		virtual void SetUniformInt(		const std::string& uniformName, I32 value) override;
		virtual void SetUniformUInt(	const std::string& uniformName, U32 value) override;
		virtual void SetUniformFloat(	const std::string& uniformName, F32 value) override;
		virtual void SetUniformFloat2(	const std::string& uniformName, const glm::vec2& value) override;
		virtual void SetUniformFloat3(	const std::string& uniformName, const glm::vec3& value) override;
		virtual void SetUniformMat2(	const std::string& uniformName, const glm::mat2& value) override;
		virtual void SetUniformMat3(	const std::string& uniformName, const glm::mat3& value) override;
		virtual void SetUniformMat4(	const std::string& uniformName, const glm::mat4& value) override;
		virtual void SetUniformBool(	const std::string& uniformName, bool value) override;
	private:
		std::map<GLuint, std::string> ExtractShaders(const std::string& source);
		void CreateShaderProgram(const std::map<GLuint, std::string> sources);

		GLint GetUniformLocation(const std::string& uniformName);

	private:
		U32 m_Id;
		std::string m_Name;
		std::unordered_map<std::string, GLint> m_UniformLocations;
	};
}