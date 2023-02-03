#pragma once

#include "Engine/Core/Types.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "Engine/Common/Geometry2D.h"

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;

    struct AABB2D;
    struct CircleBounds2D;
    struct Transform2D;
    struct Engine::LineSegment2D;
    struct Engine::Line2D;
    class EdgeCollider2D;
    class PolygonCollider2D;

    struct SATQuery
    {
        F32 Distance = -std::numeric_limits<F32>::max();
        I32 FaceIndex = -1;
    };

    bool Intersects(const AABB2D& first, const AABB2D& second);
    bool Intersects(const CircleBounds2D& first, const CircleBounds2D& second);
    bool Intersects(const AABB2D& box, const CircleBounds2D& circle);
    bool Intersects(const CircleBounds2D& circle, const AABB2D& box);

    bool Contains(const AABB2D& first, const AABB2D& second);
    bool Contains(const CircleBounds2D& first, const CircleBounds2D& second);
    bool Contains(const AABB2D& box, const CircleBounds2D& circle);
    bool Contains(const CircleBounds2D& circle, const AABB2D& box);

    bool AABBCollision2D(const AABB2D& first, const AABB2D& second);

    bool CircleCollision2D(const CircleBounds2D& first, const CircleBounds2D& second);

    bool AABBCircleCollision2D(const AABB2D& box, const CircleBounds2D& circle);

    bool CircleBoxCollision2D(const CircleBounds2D& circle, const AABB2D& box);

    bool AABBContain2D(const AABB2D& first, const AABB2D& second);

    bool CircleContain2D(const CircleBounds2D& first, const CircleBounds2D& second);

    bool AABBCircleContain2D(const AABB2D& box, const CircleBounds2D& circle);

    bool CircleAABBContain2D(const CircleBounds2D& circle, const AABB2D& box);

    // Returns the support of the box in direction of the axis, the axis is expected to be normalized.
    glm::vec2 GetSupport(const PolygonCollider2D& polygon, const glm::vec2& axis, const Transform2D& transform);
    // Performs SAT query, returns the largest distance and associated vertex.
    SATQuery SATFaceDirections(const PolygonCollider2D& first, const PolygonCollider2D& second,
                               const Transform2D& tfA, const Transform2D& tfB);
    // Returns the index of face in box `seekBox` that is incident to `refFace` (in global coordinates).
    I32 FindIncidentFaceIndex(const PolygonCollider2D& seekPolygon,
                              const glm::vec2& refFace,
                              const Transform2D& seekTf);
    // Performs the clipping of line segment by line, returns the number of points in resulting line segment.
    U32 ClipLineSegmentToLine(LineSegment2D& clippingResult, const LineSegment2D& lineSegment, const Line2D& line);
}
