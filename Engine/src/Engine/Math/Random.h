#pragma once

#include "Engine/Core/Types.h"

#include <random>
#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;
	class Random
	{
	public:
		static F32 Float();
		static F32 Float(F32 left, F32 right);
		static glm::vec2 Float2();
		static glm::vec2 Float2(F32 left, F32 right);
		static glm::vec3 Float3();
		static glm::vec3 Float3(F32 left, F32 right);
		static glm::vec4 Float4();
		static glm::vec4 Float4(F32 left, F32 right);

		static I32 Int32();
		static I32 Int32(I32 left, I32 right);
		static U32 UInt32();
		static U32 UInt32(U32 left, U32 right);
		static I64 Int64();
		static I64 Int64(I64 left, I64 right);
		static U64 UInt64();
		static U64 UInt64(U64 left, U64 right);
		
		private:
		static std::random_device m_Device;
		static std::mt19937 m_Mt;
		static std::uniform_real_distribution<> m_UniformNormalizedReal;
	};
}
