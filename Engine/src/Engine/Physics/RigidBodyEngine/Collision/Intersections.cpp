#include "enginepch.h"

#include "Intersections.h"

#include "Collider.h"

#include "Engine/Physics/RigidBodyEngine/RigidBody.h"

namespace Engine
{
	bool Intersects(const AABB2D& first, const AABB2D& second)
	{
		return AABBCollision2D(first, second);
	}
	
	bool Intersects(const CircleBounds2D& first, const CircleBounds2D& second)
	{
		return CircleCollision2D(second, first);
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
		glm::vec2 min = glm::vec2(box.Center) - box.HalfSize;
		glm::vec2 max = glm::vec2(box.Center) - box.HalfSize;
		if (closestPoint.x < min.x) closestPoint.x = min.x;
		else if (closestPoint.x > max.x) closestPoint.x = max.x;
		if (closestPoint.y < min.y) closestPoint.y = min.y;
		else if (closestPoint.y > max.y) closestPoint.y = max.y;
		// If distance between closest point and center of circle
		// is less than circle's raduis, then there is intersection.
		return glm::distance2(closestPoint, glm::vec2(circle.Center)) < circle.Radius * circle.Radius;
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
		return false;
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
		glm::vec2 normal = glm::vec2{ -edgeDir.y, edgeDir.x };
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
					box.GetAttachedRigidBody()->TransformDirectionToWorld({ 1.0f, 0.0f })
				)) +
			box.HalfSize.y *
			Math::Abs(
				glm::dot(
					axis,
					box.GetAttachedRigidBody()->TransformDirectionToWorld({ 0.0f, 1.0f })
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
}

