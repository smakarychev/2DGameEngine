#pragma once

#include "Engine/Core/Types.h"

#include "MathUtils.h"

#include <glm/glm.hpp>

namespace Engine
{
	
	using namespace Types;
	namespace Math
	{
		static F32 Cross2D(const glm::vec2& a, const glm::vec2& b)
		{
			return a.x * b.y - a.y * b.x;
		}

		static glm::vec2 Cross2D(const glm::vec2& vec, F32 val)
		{
			return { vec.y * val, -vec.x * val };
		}

		static glm::vec2 Cross2D(F32 val, const glm::vec2& vec)
		{
			return { -vec.y * val, vec.x * val };
		}
	}

}
