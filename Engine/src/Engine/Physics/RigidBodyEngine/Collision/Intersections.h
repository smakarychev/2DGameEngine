#pragma once

#include "Engine/Core/Types.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "Engine/Common/Geometry2D.h"

namespace Engine
{
    namespace Component
    {
        struct LocalToWorldTransform2D;
    }
}

namespace Engine::Physics
{
    using namespace Types;

    struct AABB2D;
    struct CircleBounds2D;
    struct Component::LocalToWorldTransform2D;
    struct Engine::LineSegment2D;
    struct Engine::Line2D;
    class BoxCollider2D;
    class EdgeCollider2D;

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

    bool BoxHalfSpaceCollision2D(const BoxCollider2D& box, const EdgeCollider2D& edge);

    // Axis is expected to be normalized.
    F32 BoxBoxOnAxisOverlap2D(const BoxCollider2D& first, const BoxCollider2D& second, const glm::vec2& axis);
    F32 BoxBoxOnAxisOverlap2D(const BoxCollider2D& first, const BoxCollider2D& second, const glm::vec2& axis,
                              const glm::vec2& distanceVec);

    // Returns the support of the box in direction of the axis, the axis is expected to be normalized.
    glm::vec2 GetSupport(const BoxCollider2D& box, const glm::vec2& axis, const Component::LocalToWorldTransform2D& transform);
    // Performs SAT query, returns the largest distance and associated vertex.
    SATQuery SATFaceDirections(const BoxCollider2D& first, const BoxCollider2D& second,
                               const Component::LocalToWorldTransform2D& tfA, const Component::LocalToWorldTransform2D& tfB);
    // Returns the index of face in box `seekBox` that is incident to `refFace` (in global coordinates).
    I32 FindIncidentFaceIndex(const BoxCollider2D& seekBox,
                              const glm::vec2& refFace,
                              const Component::LocalToWorldTransform2D& seekTf);
    // Performs the clipping of line segment by line, returns the number of points in resulting line segment.
    U32 ClipLineSegmentToLine(LineSegment2D& clippingResult, const LineSegment2D& lineSegment, const Line2D& line);
}
