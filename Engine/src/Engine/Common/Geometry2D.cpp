#include "enginepch.h"

#include "Geometry2D.h"

#include <glm/gtx/norm.hpp>

#include "Engine/Math/LinearAlgebra.h"
#include "Engine/Math/MathUtils.h"

namespace Engine
{
    Rotation::Rotation() = default;

    Rotation::Rotation(const glm::vec2& rotation): RotationVec(rotation)
    {}

    Rotation::Rotation(F32 angleRad): RotationVec(glm::cos(angleRad), glm::sin(angleRad))
    {}

    Rotation Rotation::operator-() const
    { return Rotation{glm::vec2{RotationVec.x, -RotationVec.y}}; }

    Rotation::operator const glm::vec<2, float, glm::packed_highp>&() const
    { return RotationVec; }

    Rotation::operator glm::vec<2, float, glm::packed_highp>&()
    { return RotationVec; }

    F32& Rotation::operator[](I32 index)
    { return RotationVec[index]; }

    const F32& Rotation::operator[](I32 index) const
    { return RotationVec[index]; }

    CRect::CRect(const glm::vec2& center, const glm::vec2& halfSize): Center(center), HalfSize(halfSize)
    {}

    bool CRect::Contains(const glm::vec2& point) const
    {
        return Math::Abs(point.x - Center.x) < HalfSize.x &&
            Math::Abs(point.y - Center.y) < HalfSize.y;
    }

    bool CRect::Contains(const CRect& rect) const
    {
        return Math::Abs(rect.Center.x - Center.x) < (HalfSize.x - rect.HalfSize.x) &&
            Math::Abs(rect.Center.y - Center.y) < (HalfSize.y - rect.HalfSize.y);
    }

    bool CRect::Overlaps(const CRect& rect) const
    {
        return Math::Abs(rect.Center.x - Center.x) < (HalfSize.x + rect.HalfSize.x) &&
            Math::Abs(rect.Center.y - Center.y) < (HalfSize.y + rect.HalfSize.y);
    }

    glm::vec2 TransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate,
        const glm::vec2& scale)
    {
        return Math::Rotate(scale * point, rotate) + translate;
    }

    glm::vec2 TransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate)
    {
        return Math::Rotate(point, rotate) + translate;
    }

    glm::vec2 TransformDir2D(const glm::vec2& dir, const Rotation& rotate)
    {
        return Math::Rotate(dir, rotate);
    }

    glm::vec2 InverseTransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate,
        const glm::vec2& scale)
    {
        return Math::Rotate((point - translate), -rotate) / scale;
    }

    glm::vec2 InverseTransformPoint2D(const glm::vec2& point, const glm::vec2& translate, const Rotation& rotate)
    {
        return Math::Rotate(point - translate, -rotate);
    }

    glm::vec2 InverseTransformDir2D(const glm::vec2& dir, const Rotation& rotate)
    {
        return Math::Rotate(dir, -rotate);
    }

    std::vector<glm::vec2> ConvexHull(const std::vector<glm::vec2>& points)
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
