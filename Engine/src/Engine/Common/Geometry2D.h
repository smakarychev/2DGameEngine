#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"
#include "Engine/Math/MathUtils.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "Engine/Math/LinearAlgebra.h"

namespace Engine
{
	using namespace Types;

	struct Rotation
	{
		glm::vec2 RotationVec{1.0f, 0.0f};

		Rotation() = default;
		Rotation(const glm::vec2& rotation) : RotationVec(rotation) {}
		Rotation(F32 angleRad) : RotationVec(glm::cos(angleRad), glm::sin(angleRad)) {}

		Rotation operator-() const { return Rotation{glm::vec2{RotationVec.x, -RotationVec.y}}; }
		
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

	inline glm::vec2 TransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate, const glm::vec2& scale)
	{
		return Math::Rotate(scale * point, rotate) + translate;
	}

	inline glm::vec2 TransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate)
	{
		return Math::Rotate(point, rotate) + translate;
	}

	inline glm::vec2 TransformDir2D(const glm::vec2& dir, const Rotation& rotate)
	{
		return Math::Rotate(dir, rotate);
	}

	inline glm::vec2 InverseTransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate, const glm::vec2& scale)
	{
		return Math::Rotate((point - translate), -rotate) / scale;
	}

	inline glm::vec2 InverseTransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate)
	{
		return Math::Rotate(point - translate, -rotate);
	}

	inline glm::vec2 InverseTransformDir2D(const glm::vec2& dir, const Rotation& rotate)
	{
		return Math::Rotate(dir, -rotate);
	}

	inline std::vector<glm::vec2> ConvexHull(const std::vector<glm::vec2>& points)
	{
		// Graham scan.
		// Find the bottom-left point.
		glm::vec2 bottomLeft{std::numeric_limits<F32>::max()};
		for (auto& p : points) if (p.y < bottomLeft.y || Math::CompareEqual(p.y, bottomLeft.y) && p.x < bottomLeft.x) bottomLeft = p;
		// Sort points by polar angle.
		enum class Orientation { Collinear, CW, CCW };
		auto orientation = [](const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
		{
			auto val = (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
			if (Math::CompareEqual(val, 0.0f)) return Orientation::Collinear;
			if (val < 0) return Orientation::CW;
			return Orientation::CCW;
		};
		auto polarCompare = [&](const glm::vec2& a, const glm::vec2& b)
		{
			Orientation orient = orientation(bottomLeft, a, b);
			switch (orient)
			{
			case Orientation::Collinear: return (glm::distance2(a, bottomLeft) <= glm::distance2(b, bottomLeft));
			case Orientation::CW: return false;
			case Orientation::CCW: return true;
			}
			return false;
		};
		auto sortedPoints = points;
		std::vector<glm::vec2> finalPointsModification {bottomLeft};
		std::ranges::sort(sortedPoints, polarCompare);
		// Remove collinear.
		for (U32 spI = 1; spI < sortedPoints.size() - 1; spI++)
		{
			if (orientation(finalPointsModification.front(), sortedPoints[spI], sortedPoints[spI + 1]) != Orientation::Collinear)
			{
				finalPointsModification.push_back(sortedPoints[spI]);
			}
		}
		finalPointsModification.push_back(sortedPoints.back());
		if (finalPointsModification.size() < 3) return {};		

		// Form convex hull.
		std::vector<glm::vec2> hull;
		for (auto& p : finalPointsModification)
		{
			while(hull.size() > 1 && orientation(hull[hull.size() - 2], hull.back(), p) != Orientation::CCW) hull.pop_back();
			hull.push_back(p);
		}
		
		return hull;
	}
	
}
