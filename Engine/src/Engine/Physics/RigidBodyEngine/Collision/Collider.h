#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Math/MathUtils.h"

#include "Intersections.h"

#include <vector>
#include <glm/glm.hpp>

namespace Engine
{
	template <typename Impl>
	struct Collider2D
	{
		template <typename OtherImpl>
		bool Intersects(const OtherImpl& other) const
		{
			return Engine::Intersect(static_cast<Impl&>(*this), other);
		}
	};

	struct BoxCollider2D : public Collider2D<BoxCollider2D>
	{
		glm::vec3 Center;
		glm::vec2 HalfSize;
		BoxCollider2D(const glm::vec3& center = glm::vec3{ 0.0f }, const glm::vec2& halfSize = glm::vec2{ 1.0f })
			: Center(center), HalfSize(halfSize)
		{}
		BoxCollider2D(const BoxCollider2D& first, const BoxCollider2D& second)
		{
			ENGINE_CORE_ASSERT(first.Center.z == second.Center.z, "Boxes exist on the different planes.");
			glm::vec2 min = {
				Math::Min(first.Center.x - first.HalfSize.x, second.Center.x - second.HalfSize.x),
				Math::Min(first.Center.y - first.HalfSize.y, second.Center.y - second.HalfSize.y)
			};
			glm::vec2 max = {
				Math::Max(first.Center.x + first.HalfSize.x, second.Center.x + second.HalfSize.x),
				Math::Max(first.Center.y + first.HalfSize.y, second.Center.y + second.HalfSize.y)
			};
			Center = glm::vec3((min + max) * 0.5f, first.Center.z);
			HalfSize = (min - max) * 0.5f;
		}

		void Expand(const glm::vec2& expansion)
		{
			HalfSize += expansion;
		}

		F32 GetPerimeter() const
		{
			return 4.0f * (HalfSize.x + HalfSize.y);
		}
	};

	struct CircleCollider2D : public Collider2D<CircleCollider2D>
	{
		glm::vec3 Center;
		F32 Radius;
		CircleCollider2D(const glm::vec3& center = glm::vec3{0.0f}, F32 radius = 1.0f)
			: Center(center), Radius(radius)
		{}
		CircleCollider2D(const CircleCollider2D& first, const CircleCollider2D& second)
		{
			ENGINE_CORE_ASSERT(first.Center.z == second.Center.z, "Circles exist on the different planes.");
			if (CircleCircleContain2D(first, second))
			{
				const CircleCollider2D& biggest = first.Radius > second.Radius ? first : second;
				Center = biggest.Center;
				Radius = biggest.Radius;
				return;
			}
			glm::vec3 centerOffset = second.Center - first.Center;
			F32 distance = glm::distance(first.Center, second.Center);
			Radius = (first.Radius + second.Radius + distance) * 0.5f;
			Center = first.Center;
			if (distance > 0)
			{
				Center += centerOffset * (Radius - first.Radius) / distance;
			}
		}

		void Expand(const glm::vec2& expansion)
		{
			Radius += Math::Max(expansion.x, expansion.y);
		}

		F32 GetPerimeter() const
		{
			return 2.0f * Math::Pi<F32>() * Radius;
		}
	};
}
