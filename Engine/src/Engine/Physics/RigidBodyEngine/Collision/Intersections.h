#pragma once

#include <glm/gtx/norm.hpp>

namespace Engine
{
	struct BoxCollider2D;
	struct CircleCollider2D;

	bool Intersects(const BoxCollider2D& first, const BoxCollider2D& second);
	bool Intersects(const CircleCollider2D& first, const CircleCollider2D& second);
	bool Intersects(const BoxCollider2D& box, const CircleCollider2D& circle);
	bool Intersects(const CircleCollider2D& circle, const BoxCollider2D& box);

	bool Contains(const BoxCollider2D& first, const BoxCollider2D& second);
	bool Contains(const CircleCollider2D& first, const CircleCollider2D& second);
	bool Contains(const BoxCollider2D& box, const CircleCollider2D& circle);
	bool Contains(const CircleCollider2D& circle, const BoxCollider2D& box);

	bool BoxBoxCollision2D(const BoxCollider2D& first, const BoxCollider2D& second);

	bool CircleCircleCollision2D(const CircleCollider2D& first, const CircleCollider2D& second);

	bool BoxCircleCollision2D(const BoxCollider2D& box, const CircleCollider2D& circle);
	
	bool CircleBoxCollision2D(const CircleCollider2D& circle, const BoxCollider2D& box);

	bool BoxBoxContain2D(const BoxCollider2D& first, const BoxCollider2D& second);

	bool CircleCircleContain2D(const CircleCollider2D& first, const CircleCollider2D& second);

	bool BoxCircleContain2D(const BoxCollider2D& box, const CircleCollider2D& circle);

	bool CircleBoxContain2D(const CircleCollider2D& circle, const BoxCollider2D& box);
}
