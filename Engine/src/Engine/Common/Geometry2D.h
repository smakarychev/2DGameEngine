#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"
#include "Engine/Math/MathUtils.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;

	struct Rect
	{
		glm::vec2 Center;
		glm::vec2 HalfSize;
		Rect(const glm::vec2& center = glm::vec2{ 0.0f }, const glm::vec2& halfSize = glm::vec2{ 1.0f }) : Center(center), HalfSize(halfSize)
		{}

		bool Contains(const glm::vec2& point) const
		{
			return Math::Abs(point.x - Center.x) < HalfSize.x &&
				Math::Abs(point.y - Center.y) < HalfSize.y;
		}

		bool Contains(const Rect& rect) const
		{
			return Math::Abs(rect.Center.x - Center.x) < (HalfSize.x - rect.HalfSize.x) &&
				Math::Abs(rect.Center.y - Center.y) < (HalfSize.y - rect.HalfSize.y);
		}

		bool Overlaps(const Rect& rect) const
		{
			return Math::Abs(rect.Center.x - Center.x) < (HalfSize.x + rect.HalfSize.x) &&
				Math::Abs(rect.Center.y - Center.y) < (HalfSize.y + rect.HalfSize.y);
		}

	};
}