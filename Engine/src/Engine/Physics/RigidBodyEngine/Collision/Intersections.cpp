#include "enginepch.h"

#include "Intersections.h"

#include "Collider.h"


namespace Engine
{
	bool Intersect(const BoxCollider2D& first, const BoxCollider2D& second)
	{
		return BoxBoxCollision2D(first, second);
	}
	
	bool Intersect(const CircleCollider2D& first, const CircleCollider2D& second)
	{
		return CircleCircleCollision2D(second, first);
	}
	
	bool Intersect(const BoxCollider2D& box, const CircleCollider2D& circle)
	{
		return BoxCircleCollision2D(box, circle);
	}
	
	bool Intersect(const CircleCollider2D& circle, const BoxCollider2D& box)
	{
		return BoxCircleCollision2D(box, circle);
	}

	bool Contain(const BoxCollider2D& first, const BoxCollider2D& second)
	{
		return BoxBoxContain2D(first, second);
	}

	bool Contain(const CircleCollider2D& first, const CircleCollider2D& second)
	{
		return CircleCircleContain2D(first, second);
	}

	bool Contain(const BoxCollider2D& box, const CircleCollider2D& circle)
	{
		return BoxCircleContain2D(box, circle);
	}

	bool Contain(const CircleCollider2D& circle, const BoxCollider2D& box)
	{
		return CircleBoxContain2D(circle, box);
	}

	bool BoxBoxCollision2D(const BoxCollider2D& first, const BoxCollider2D& second)
	{
		if (first.Center.z != second.Center.z) return false;
		return
			Math::Abs(first.Center.x - second.Center.x) < (first.HalfSize.x + second.HalfSize.x) &&
			Math::Abs(first.Center.y - second.Center.y) < (first.HalfSize.y + second.HalfSize.y);
	}

	bool CircleCircleCollision2D(const CircleCollider2D& first, const CircleCollider2D& second)
	{
		if (first.Center.z != second.Center.z) return false;
		glm::vec2 distanceVec = first.Center - second.Center;
		F32 distanceSquared = glm::length2(distanceVec);
		F32 radSquared = (first.Radius - second.Radius) * (first.Radius - second.Radius);
		return distanceSquared < radSquared;
	}

	bool BoxCircleCollision2D(const BoxCollider2D& box, const CircleCollider2D& circle)
	{
		if (box.Center.z != circle.Center.z) return false;
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

	bool CircleBoxCollision2D(const CircleCollider2D& circle, const BoxCollider2D& box)
	{
		return BoxCircleCollision2D(box, circle);
	}

	bool BoxBoxContain2D(const BoxCollider2D& first, const BoxCollider2D& second)
	{
		if (first.Center.z != second.Center.z) return false;
		return
			Math::Abs(first.Center.x - second.Center.x) < (first.HalfSize.x - second.HalfSize.x) &&
			Math::Abs(first.Center.y - second.Center.y) < (first.HalfSize.y - second.HalfSize.y);
	}

	bool CircleCircleContain2D(const CircleCollider2D& first, const CircleCollider2D& second)
	{
		if (first.Center.z != second.Center.z) return false;
		F32 radDiff = Math::Abs(first.Radius - second.Radius);
		return
			glm::distance2(first.Center, second.Center) < radDiff * radDiff;
	}

	bool BoxCircleContain2D(const BoxCollider2D& box, const CircleCollider2D& circle)
	{
		if (box.Center.z != circle.Center.z) return false;
		return
			Math::Abs(box.Center.x - circle.Center.x) + circle.Radius < box.HalfSize.x &&
			Math::Abs(box.Center.y - circle.Center.y) + circle.Radius < box.HalfSize.y;
		return false;
	}

	bool CircleBoxContain2D(const CircleCollider2D& circle, const BoxCollider2D& box)
	{
		if (box.Center.z != circle.Center.z) return false;
		return
			Math::Abs(box.Center.x - circle.Center.x) + box.HalfSize.x < circle.Radius &&
			Math::Abs(box.Center.y - circle.Center.y) + box.HalfSize.y < circle.Radius;
	}
}

