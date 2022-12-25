#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"
#include "Engine/Math/MathUtils.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;

	struct Rotation
	{
		glm::vec2 RotationVec;

		Rotation(const glm::vec2& rotation) : RotationVec(rotation)
		{
		}

		Rotation(F32 angleRad) : RotationVec(glm::cos(angleRad), glm::sin(angleRad))
		{
		}

		operator const glm::vec2&() const { return RotationVec; }
		operator glm::vec2&() { return RotationVec; }
		F32& operator[](I32 index) { return RotationVec[index]; }
		const F32& operator[](I32 index) const { return RotationVec[index]; }
	};
	
	struct CRect
	{
		glm::vec2 Center;
		glm::vec2 HalfSize;
		CRect(const glm::vec2& center = glm::vec2{ 0.0f }, const glm::vec2& halfSize = glm::vec2{ 1.0f }) : Center(center), HalfSize(halfSize)
		{}

		bool Contains(const glm::vec2& point) const
		{
			return Math::Abs(point.x - Center.x) < HalfSize.x &&
				Math::Abs(point.y - Center.y) < HalfSize.y;
		}

		bool Contains(const CRect& rect) const
		{
			return Math::Abs(rect.Center.x - Center.x) < (HalfSize.x - rect.HalfSize.x) &&
				Math::Abs(rect.Center.y - Center.y) < (HalfSize.y - rect.HalfSize.y);
		}

		bool Overlaps(const CRect& rect) const
		{
			return Math::Abs(rect.Center.x - Center.x) < (HalfSize.x + rect.HalfSize.x) &&
				Math::Abs(rect.Center.y - Center.y) < (HalfSize.y + rect.HalfSize.y);
		}
	};

	struct Line2D
	{
		F32 Offset = 0.0f;
		glm::vec2 Normal = glm::vec2{ 1.0f, 0.0f };
	};

	struct LineSegment2D
	{
		glm::vec2 Start = glm::vec2{ -1.0f, 0.0f };
		glm::vec2 End   = glm::vec2{ 1.0f, 0.0f };
	};
}