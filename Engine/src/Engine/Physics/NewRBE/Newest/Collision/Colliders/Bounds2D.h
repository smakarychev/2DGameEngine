#pragma once

#include "../Intersections.h"
#include "Engine/Math/MathUtils.h"

namespace Engine::WIP::Physics::Newest
{
    // Represents the bounds of some collider, collider therefore has an ability to create a bounds for itself.
	template <typename Impl>
	struct Bounds2D
	{
		template <typename OtherImpl>
		bool Intersects(const OtherImpl& other) const;
		template <typename OtherImpl>
		bool Contains(const OtherImpl& other) const;
	};

	struct AABB2D : Bounds2D<AABB2D>
	{
		glm::vec2 Center{};
		glm::vec2 HalfSize{};
		AABB2D(const glm::vec2& center = glm::vec2{ 0.0f }, const glm::vec2& halfSize = glm::vec2{ 0.0f });
		AABB2D(const AABB2D& first, const AABB2D& second);

		static AABB2D GetFromMinMax(const glm::vec2& min, const glm::vec2& max);
		void Expand(const glm::vec2& expansion);
		void ExpandSigned(const glm::vec2& signedExpansion);
		F32 GetPerimeter() const;
		AABB2D Translate(const glm::vec2& pos) const;
		AABB2D Rotate(const glm::vec2& rotVec) const;
	};

	struct CircleBounds2D : Bounds2D<CircleBounds2D>
	{
		glm::vec2 Center{};
		F32 Radius{};
		CircleBounds2D(const glm::vec2& center = glm::vec2{0.0f}, F32 radius = 0.0f);
		CircleBounds2D(const CircleBounds2D& first, const CircleBounds2D& second);
		void Expand(const glm::vec2& expansion);
		void EncapsulatePoint(const glm::vec2& point);
		F32 GetPerimeter() const;
		CircleBounds2D Translate(const glm::vec2& pos) const;
		CircleBounds2D Rotate(const glm::vec2& rotVec) const;
	};

	template <typename Impl>
	template <typename OtherImpl>
	bool Bounds2D<Impl>::Intersects(const OtherImpl& other) const
	{
		return Physics::Newest::Intersects(*reinterpret_cast<const Impl*>(this), other);
	}

	template <typename Impl>
	template <typename OtherImpl>
	bool Bounds2D<Impl>::Contains(const OtherImpl& other) const
	{
		return Physics::Newest::Contains(*reinterpret_cast<const Impl*>(this), other);
	}
}
