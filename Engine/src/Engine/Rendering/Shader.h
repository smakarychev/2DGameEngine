#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <filesystem>

namespace Engine
{
	using namespace Types;

	enum class ShaderType
	{
		Vertex, Pixel, Hull, Domain, Geometry
	};

	enum class ShaderUniformType
	{
		Float, Float2, Float3, Float4,
		Mat2, Mat3, Mat4,
		Int, UInt,
		Bool
	};

	class Shader
	{
	public:
		virtual ~Shader() {}
		static Ref<Shader> ReadShaderFromFile(const std::filesystem::path& path);
		static Ref<Shader> CreateShaderFromSource(const std::string& name, const std::string& source);

		virtual U32 GetId() const = 0;

		virtual void Bind() = 0;

		virtual void SetUniform(		const std::string& uniformName, void* value, ShaderUniformType type) = 0;
		virtual void SetUniformInt(		const std::string& uniformName, I32 value) = 0;
		virtual void SetUniformUInt(	const std::string& uniformName, U32 value) = 0;
		virtual void SetUniformFloat(	const std::string& uniformName, F32 value) = 0;
		virtual void SetUniformFloat2(	const std::string& uniformName, const glm::vec2& value) = 0;
		virtual void SetUniformFloat3(	const std::string& uniformName, const glm::vec3& value) = 0;
		virtual void SetUniformMat2(	const std::string& uniformName, const glm::mat2& value) = 0;
		virtual void SetUniformMat3(	const std::string& uniformName, const glm::mat3& value) = 0;
		virtual void SetUniformMat4(	const std::string& uniformName, const glm::mat4& value) = 0;
		virtual void SetUniformBool(	const std::string& uniformName, bool value) = 0;
	};
}