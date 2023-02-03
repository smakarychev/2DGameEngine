#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;

	struct Rotation
	{
		glm::vec2 RotationVec{1.0f, 0.0f};

		Rotation();
		Rotation(const glm::vec2& rotation);
		Rotation(F32 angleRad);

		Rotation operator-() const;

		operator const glm::vec2&() const;
		operator glm::vec2&();
		F32& operator[](I32 index);
		const F32& operator[](I32 index) const;
	};
	
	struct CRect
	{
		glm::vec2 Center;
		glm::vec2 HalfSize;
		CRect(const glm::vec2& center = glm::vec2{ 0.0f }, const glm::vec2& halfSize = glm::vec2{ 1.0f });

		bool Contains(const glm::vec2& point) const;

		bool Contains(const CRect& rect) const;

		bool Overlaps(const CRect& rect) const;
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

	glm::vec2 TransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate, const glm::vec2& scale);

	glm::vec2 TransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate);

	glm::vec2 TransformDir2D(const glm::vec2& dir, const Rotation& rotate);

	glm::vec2 InverseTransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate, const glm::vec2& scale);

	glm::vec2 InverseTransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate);

	glm::vec2 InverseTransformDir2D(const glm::vec2& dir, const Rotation& rotate);

	std::vector<glm::vec2> ConvexHull(const std::vector<glm::vec2>& points);
}
