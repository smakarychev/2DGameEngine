#include "enginepch.h"

#include "Intersections.h"

#include "Collider.h"
#include "Engine/ECS/Components.h"

#include "Engine/Physics/RigidBodyEngine/RigidBody.h"

namespace Engine::Physics
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

    bool BoxHalfSpaceCollision2D(const BoxCollider2D& box, const EdgeCollider2D& edge)
    {
        // Convert to world space.
        glm::vec2 boxCenter = box.GetAttachedRigidBody()->TransformToWorld(box.Center);
        glm::vec2 edgeStart = edge.GetAttachedRigidBody()->TransformToWorld(edge.Start);
        glm::vec2 edgeEnd = edge.GetAttachedRigidBody()->TransformToWorld(edge.End);

        // Get space normal.
        glm::vec2 edgeDir = edgeEnd - edgeStart;
        glm::vec2 normal = glm::vec2{-edgeDir.y, edgeDir.x};
        normal = glm::normalize(normal);
        F32 offset = -glm::dot(normal, edgeStart);
        // Compute the projection interval raduis.
        F32 projection = box.HalfSize.x * Math::Abs(normal.x) + box.HalfSize.y * Math::Abs(normal.y);
        // Center-edge distance.
        F32 distance = glm::dot(normal, boxCenter) - offset;

        return Math::Abs(distance) < projection;
    }

    F32 TransformToAxis(const BoxCollider2D& box, const glm::vec2& axis)
    {
        return
            box.HalfSize.x *
            Math::Abs(
                glm::dot(
                    axis,
                    box.GetAttachedRigidBody()->TransformDirectionToWorld({1.0f, 0.0f})
                )) +
            box.HalfSize.y *
            Math::Abs(
                glm::dot(
                    axis,
                    box.GetAttachedRigidBody()->TransformDirectionToWorld({0.0f, 1.0f})
                ));
    }

    F32 BoxBoxOnAxisOverlap2D(const BoxCollider2D& first, const BoxCollider2D& second, const glm::vec2& axis)
    {
        // Project half sizes and compare to distance.
        F32 firstSizeProj = TransformToAxis(first, axis);
        F32 secondSizeProj = TransformToAxis(second, axis);
        glm::vec2 firstCenter = first.GetAttachedRigidBody()->TransformToWorld(first.Center);
        glm::vec2 secondCenter = second.GetAttachedRigidBody()->TransformToWorld(second.Center);
        glm::vec2 distanceVec = firstCenter - secondCenter;
        F32 distance = Math::Abs(glm::dot(distanceVec, axis));
        return firstSizeProj + secondSizeProj - distance;
    }

    F32 BoxBoxOnAxisOverlap2D(const BoxCollider2D& first, const BoxCollider2D& second, const glm::vec2& axis,
                              const glm::vec2& distanceVec)
    {
        // Project half sizes and compare to distance.
        F32 firstSizeProj = TransformToAxis(first, axis);
        F32 secondSizeProj = TransformToAxis(second, axis);
        F32 distance = Math::Abs(glm::dot(distanceVec, axis));
        return firstSizeProj + secondSizeProj - distance;
    }

    glm::vec2 GetSupport(const BoxCollider2D& box, const glm::vec2& axis, const Component::LocalToWorldTransform2D& transform)
    {
        std::array<glm::vec2, 4> verticesLocal{
            glm::vec2{box.Center.x - box.HalfSize.x, box.Center.y - box.HalfSize.y},
            glm::vec2{box.Center.x + box.HalfSize.x, box.Center.y - box.HalfSize.y},
            glm::vec2{box.Center.x + box.HalfSize.x, box.Center.y + box.HalfSize.y},
            glm::vec2{box.Center.x - box.HalfSize.x, box.Center.y + box.HalfSize.y},
        };
        F32 bestProjection = -std::numeric_limits<F32>::max();
        glm::vec2 bestVertex = {bestProjection, bestProjection};
        for (U32 i = 0; i < verticesLocal.size(); i++)
        {
            glm::vec2 vertex = transform.Transform(verticesLocal[i]);
            F32 projection = glm::dot(vertex, axis);
            if (projection > bestProjection)
            {
                bestProjection = projection;
                bestVertex = vertex;
            }
        }
        return bestVertex;
    }

    SATQuery SATFaceDirections(const BoxCollider2D& first, const BoxCollider2D& second,
                               const Component::LocalToWorldTransform2D& tfA, const Component::LocalToWorldTransform2D& tfB)
    {
        constexpr std::array<glm::vec2, 4> normalsLocal{
            glm::vec2{0.0f, -1.0f},
            glm::vec2{1.0f, 0.0f},
            glm::vec2{0.0f, 1.0f},
            glm::vec2{-1.0f, 0.0f},
        };

        std::array<glm::vec2, 4> verticesLocal{
            glm::vec2{first.Center.x - first.HalfSize.x, first.Center.y - first.HalfSize.y},
            glm::vec2{first.Center.x + first.HalfSize.x, first.Center.y - first.HalfSize.y},
            glm::vec2{first.Center.x + first.HalfSize.x, first.Center.y + first.HalfSize.y},
            glm::vec2{first.Center.x - first.HalfSize.x, first.Center.y + first.HalfSize.y},
        };

        F32 bestDistance = -std::numeric_limits<F32>::max();
        I32 bestFaceI = -1;
        for (U32 i = 0; i < normalsLocal.size(); i++)
        {
            glm::vec2 normal = tfA.TransformDirection(normalsLocal[i]);
            glm::vec2 supportVec = GetSupport(second, -normal, tfB);
            // Distance between vertex and plane with dir of normal.
            F32 planeOffset = -glm::dot(normal, tfA.Transform(verticesLocal[i]));
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

    I32 FindIncidentFaceIndex(const BoxCollider2D& seekBox,
                              const glm::vec2& refFace,
                              const Component::LocalToWorldTransform2D& seekTf)
    {
        constexpr std::array<glm::vec2, 4> normalsLocal{
            glm::vec2{0.0f, -1.0f},
            glm::vec2{1.0f, 0.0f},
            glm::vec2{0.0f, 1.0f},
            glm::vec2{-1.0f, 0.0f},
        };

        glm::vec2 refNormal{-refFace.y, refFace.x};
        F32 bestDot = std::numeric_limits<F32>::max();
        I32 bestFaceI = -1;
        for (U32 i = 0; i < normalsLocal.size(); i++)
        {
            glm::vec2 normal = seekTf.TransformDirection(normalsLocal[i]);
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
