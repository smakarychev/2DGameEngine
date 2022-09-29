#pragma once

#include <glm/gtx/norm.hpp>

namespace Engine
{
	struct BoxCollider2D;
	struct CircleCollider2D;

	bool Intersect(const BoxCollider2D& first, const BoxCollider2D& second);
	bool Intersect(const CircleCollider2D& first, const CircleCollider2D& second);
	bool Intersect(const BoxCollider2D& box, const CircleCollider2D& circle);
	bool Intersect(const CircleCollider2D& circle, const BoxCollider2D& box);

	bool Contain(const BoxCollider2D& first, const BoxCollider2D& second);
	bool Contain(const CircleCollider2D& first, const CircleCollider2D& second);
	bool Contain(const BoxCollider2D& box, const CircleCollider2D& circle);
	bool Contain(const CircleCollider2D& circle, const BoxCollider2D& box);

	bool BoxBoxCollision2D(const BoxCollider2D& first, const BoxCollider2D& second);

	bool CircleCircleCollision2D(const CircleCollider2D& first, const CircleCollider2D& second);

	bool BoxCircleCollision2D(const BoxCollider2D& box, const CircleCollider2D& circle);
	
	bool CircleBoxCollision2D(const CircleCollider2D& circle, const BoxCollider2D& box);

	bool BoxBoxContain2D(const BoxCollider2D& first, const BoxCollider2D& second);

	bool CircleCircleContain2D(const CircleCollider2D& first, const CircleCollider2D& second);

	bool BoxCircleContain2D(const BoxCollider2D& box, const CircleCollider2D& circle);

	bool CircleBoxContain2D(const CircleCollider2D& circle, const BoxCollider2D& box);
}
