#include "enginepch.h"

#include "Intersections.h"

#include "Collider2D.h"
#include "PolygonCollider2D.h"
#include "Engine/Physics/NewRBE/Newest/Transform.h"

namespace Engine::WIP::Physics::Newest
{
    bool Intersects(const AABB2D& first, const AABB2D& second)
    {
        return AABBCollision2D(first, second);
    }

    bool Intersects(const CircleBounds2D& first, const CircleBounds2D& second)
    {
        return CircleCollision2D(first, second);
    }

    bool Intersects(const AABB2D& box, const CircleBounds2D& circle)
    {
        return AABBCircleCollision2D(box, circle);
    }

    bool Intersects(const CircleBounds2D& circle, const AABB2D& box)
    {
        return AABBCircleCollision2D(box, circle);
    }

    bool Contains(const AABB2D& first, const AABB2D& second)
    {
        return AABBContain2D(first, second);
    }

    bool Contains(const CircleBounds2D& first, const CircleBounds2D& second)
    {
        return CircleContain2D(first, second);
    }

    bool Contains(const AABB2D& box, const CircleBounds2D& circle)
    {
        return AABBCircleContain2D(box, circle);
    }

    bool Contains(const CircleBounds2D& circle, const AABB2D& box)
    {
        return CircleAABBContain2D(circle, box);
    }

    bool AABBCollision2D(const AABB2D& first, const AABB2D& second)
    {
        return
            Math::Abs(first.Center.x - second.Center.x) < (first.HalfSize.x + second.HalfSize.x) &&
            Math::Abs(first.Center.y - second.Center.y) < (first.HalfSize.y + second.HalfSize.y);
    }

    bool CircleCollision2D(const CircleBounds2D& first, const CircleBounds2D& second)
    {
        glm::vec2 distanceVec = first.Center - second.Center;
        F32 distanceSquared = glm::length2(distanceVec);
        F32 radSquared = (first.Radius - second.Radius) * (first.Radius - second.Radius);
        return distanceSquared < radSquared;
    }

    bool AABBCircleCollision2D(const AABB2D& box, const CircleBounds2D& circle)
    {
        // First, find the closest point of the box to the circle.
        glm::vec2 closestPoint = box.Center;
        glm::vec2 min = box.Center - box.HalfSize;
        glm::vec2 max = box.Center - box.HalfSize;
        if (closestPoint.x < min.x) closestPoint.x = min.x;
        else if (closestPoint.x > max.x) closestPoint.x = max.x;
        if (closestPoint.y < min.y) closestPoint.y = min.y;
        else if (closestPoint.y > max.y) closestPoint.y = max.y;
        // If distance between closest point and center of circle
        // is less than circle's raduis, then there is intersection.
        return glm::distance2(closestPoint, circle.Center) < circle.Radius * circle.Radius;
    }

    bool CircleBoxCollision2D(const CircleBounds2D& circle, const AABB2D& box)
    {
        return AABBCircleCollision2D(box, circle);
    }

    bool AABBContain2D(const AABB2D& first, const AABB2D& second)
    {
        return
            Math::Abs(first.Center.x - second.Center.x) < (first.HalfSize.x - second.HalfSize.x) &&
            Math::Abs(first.Center.y - second.Center.y) < (first.HalfSize.y - second.HalfSize.y);
    }

    bool CircleContain2D(const CircleBounds2D& first, const CircleBounds2D& second)
    {
        F32 radDiff = Math::Abs(first.Radius - second.Radius);
        return
            glm::distance2(first.Center, second.Center) < radDiff * radDiff;
    }

    bool AABBCircleContain2D(const AABB2D& box, const CircleBounds2D& circle)
    {
        return
            Math::Abs(box.Center.x - circle.Center.x) + circle.Radius < box.HalfSize.x &&
            Math::Abs(box.Center.y - circle.Center.y) + circle.Radius < box.HalfSize.y;
    }

    bool CircleAABBContain2D(const CircleBounds2D& circle, const AABB2D& box)
    {
        return
            Math::Abs(box.Center.x - circle.Center.x) + box.HalfSize.x < circle.Radius &&
            Math::Abs(box.Center.y - circle.Center.y) + box.HalfSize.y < circle.Radius;
    }

    glm::vec2 GetSupport(const PolygonCollider2D& polygon, const glm::vec2& axis, const Transform2D& transform)
    {
        F32 bestProjection = -std::numeric_limits<F32>::max();
        glm::vec2 bestVertex = {bestProjection, bestProjection};
        for (auto& vertexLocal : polygon.GetVertices())
        {
            glm::vec2 vertex = transform.Transform(vertexLocal);
            F32 projection = glm::dot(vertex, axis);
            if (projection > bestProjection)
            {
                bestProjection = projection;
                bestVertex = vertex;
            }
        }
        return bestVertex;
    }

    SATQuery SATFaceDirections(const PolygonCollider2D& first, const PolygonCollider2D& second,
                               const Transform2D& tfA, const Transform2D& tfB)
    {
        F32 bestDistance = -std::numeric_limits<F32>::max();
        I32 bestFaceI = -1;
        for (U32 i = 0; i < first.GetNormals().size(); i++)
        {
            glm::vec2 normal = tfA.TransformDirection(first.GetNormals()[i]);
            glm::vec2 supportVec = GetSupport(second, -normal, tfB);
            // Distance between vertex and plane with dir of normal.
            F32 planeOffset = -glm::dot(normal, tfA.Transform(first.GetVertices()[i]));
            F32 distance = glm::dot(normal, supportVec) + planeOffset;
            // The largest distance is the pen depth (and is negative if there is pen).
            if (distance > bestDistance)
            {
                bestDistance = distance;
                bestFaceI = static_cast<I32>(i);
            }
        }
        return SATQuery{.Distance = bestDistance, .FaceIndex = bestFaceI};
    }

    I32 FindIncidentFaceIndex(const PolygonCollider2D& seekPolygon,
                              const glm::vec2& refFace,
                              const Transform2D& seekTf)
    {
        glm::vec2 refNormal{-refFace.y, refFace.x};
        F32 bestDot = std::numeric_limits<F32>::max();
        I32 bestFaceI = -1;
        for (U32 i = 0; i < seekPolygon.GetNormals().size(); i++)
        {
            glm::vec2 normal = seekTf.TransformDirection(seekPolygon.GetNormals()[i]);
            F32 dot = glm::dot(normal, refNormal);
            if (dot < bestDot)
            {
                bestDot = dot;
                bestFaceI = static_cast<I32>(i);
            }
        }
        return bestFaceI;
    }

    U32 ClipLineSegmentToLine(LineSegment2D& clippingResult, const LineSegment2D& lineSegment, const Line2D& line)
    {
        // Find distances from ends of lineSegment to the line.
        F32 distanceA = glm::dot(lineSegment.Start, line.Normal) + line.Offset;
        F32 distanceB = glm::dot(lineSegment.End, line.Normal) + line.Offset;

        // Else all points are clipped.
        if (distanceA > 0.0f && distanceB > 0.0f)
        {
            return 0;
        }
        else
        {
            if (distanceA * distanceB < 0.0f)
            {
                // Perform clipping.
                // Based of similiar triangles.
                if (distanceA < 0.0f)
                {
                    // `start` remains.
                    F32 clippedT = -distanceA / (distanceB - distanceA);
                    glm::vec2 clipped = lineSegment.Start + clippedT * (lineSegment.End - lineSegment.Start);
                    clippingResult.Start = lineSegment.Start;
                    clippingResult.End = clipped;
                }
                else
                {
                    // `end` remains.
                    F32 clippedT = distanceA / (-distanceB + distanceA);
                    glm::vec2 clipped = lineSegment.Start + clippedT * (lineSegment.End - lineSegment.Start);
                    clippingResult.Start = lineSegment.End;
                    clippingResult.End = clipped;
                }
            }
            else
            {
                clippingResult = lineSegment;
            }
            return 2;
        }
    }
}
