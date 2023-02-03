#include "enginepch.h"

#include "Bounds2D.h"

namespace Engine::WIP::Physics::Newest
{
    AABB2D::AABB2D(const glm::vec2& center, const glm::vec2& halfSize): Center(center), HalfSize(halfSize)
	{}

	AABB2D::AABB2D(const AABB2D& first, const AABB2D& second)
	{
		glm::vec2 min = {
			Math::Min(first.Center.x - first.HalfSize.x, second.Center.x - second.HalfSize.x),
			Math::Min(first.Center.y - first.HalfSize.y, second.Center.y - second.HalfSize.y)
		};
		glm::vec2 max = {
			Math::Max(first.Center.x + first.HalfSize.x, second.Center.x + second.HalfSize.x),
			Math::Max(first.Center.y + first.HalfSize.y, second.Center.y + second.HalfSize.y)
		};
		Center = glm::vec2((min + max) * 0.5f);
		HalfSize = (max - min) * 0.5f;
	}

	AABB2D AABB2D::GetFromMinMax(const glm::vec2& min, const glm::vec2& max)
	{
		return AABB2D((min + max) * 0.5f, (max - min) * 0.5f);
	}

	void AABB2D::Expand(const glm::vec2& expansion)
	{
		HalfSize += expansion;
	}

	void AABB2D::ExpandSigned(const glm::vec2& signedExpansion)
	{
		glm::vec2 delta = signedExpansion * 0.5f;
		HalfSize.x += Math::Abs(delta.x);
		HalfSize.y += Math::Abs(delta.y);
		Center.x += delta.x;
		Center.y += delta.y;
	}

	F32 AABB2D::GetPerimeter() const
	{
		return 4.0f * (HalfSize.x + HalfSize.y);
	}

	CircleBounds2D::CircleBounds2D(const glm::vec2& center, F32 radius): Center(center), Radius(radius)
	{}

	CircleBounds2D::CircleBounds2D(const CircleBounds2D& first, const CircleBounds2D& second)
	{
		if (CircleContain2D(first, second))
		{
			const CircleBounds2D& biggest = first.Radius > second.Radius ? first : second;
			Center = biggest.Center;
			Radius = biggest.Radius;
			return;
		}
		const glm::vec2 centerOffset = second.Center - first.Center;
		const F32 distance = glm::length(centerOffset);
		Radius = (first.Radius + second.Radius + distance) * 0.5f;
		Center = first.Center;
		if (distance > 0)
		{
			Center += centerOffset * (Radius - first.Radius) / distance;
		}
	}

	void CircleBounds2D::Expand(const glm::vec2& expansion)
	{
		Radius += Math::Max(expansion.x, expansion.y);
	}

	void CircleBounds2D::EncapsulatePoint(const glm::vec2& point)
	{
		glm::vec2 distVec = point - Center;
		F32 distSquared = glm::length2(distVec);
		if (distSquared > Radius * Radius)
		{
			F32 dist = Math::Sqrt(distSquared);
			F32 radius = 0.5f * (Radius + dist);
			Center += (radius - Radius) / dist * distVec;
			Radius = radius;
		}
	}

	F32 CircleBounds2D::GetPerimeter() const
	{
		return 2.0f * Math::Pi<F32>() * Radius;
	}
}
