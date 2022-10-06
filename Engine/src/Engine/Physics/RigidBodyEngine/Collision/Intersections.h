#pragma once

#include "Engine/Core/Types.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace Engine
{
	using namespace Types;

	struct AABB2D;
	struct CircleBounds2D;
	class BoxCollider2D;
	class EdgeCollider2D;

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

}
