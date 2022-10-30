#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Common/Geometry2D.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Memory/MemoryManager.h"

#include "Engine/Physics/RigidBodyEngine/PhysicsMaterial.h"

#include "Intersections.h"

#include <vector>
#include <glm/glm.hpp>

/*
TODO: abandon box for general polygon?
TODO: use filtering instead of 3d position!
*/

namespace Engine
{
	// Represents the bounds of some collider,
	// collider therefore has an ability
	// to create a bounds for itself.
	template <typename Impl>
	struct Bounds2D
	{
		template <typename OtherImpl>
		bool Intersects(const OtherImpl& other) const
		{
			return Engine::Intersects(*reinterpret_cast<const Impl*>(this), other);
		}

		template <typename OtherImpl>
		bool Contains(const OtherImpl& other) const
		{
			return Engine::Contains(*reinterpret_cast<const Impl*>(this), other);
		}
	};

	struct AABB2D : public Bounds2D<AABB2D>
	{
		glm::vec2 Center;
		glm::vec2 HalfSize;
		AABB2D(const glm::vec2& center = glm::vec2{ 0.0f }, const glm::vec2& halfSize = glm::vec2{ 1.0f })
			: Center(center), HalfSize(halfSize)
		{}
		AABB2D(const AABB2D& first, const AABB2D& second)
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

		void Expand(const glm::vec2& expansion)
		{
			HalfSize += expansion;
		}

		void ExpandSigned(const glm::vec2& signedExpansion)
		{
			glm::vec2 delta = signedExpansion * 0.5f;
			HalfSize.x += Math::Abs(delta.x);
			HalfSize.y += Math::Abs(delta.y);
			Center.x += delta.x;
			Center.y += delta.y;
		}

		F32 GetPerimeter() const
		{
			return 4.0f * (HalfSize.x + HalfSize.y);
		}
	};

	struct CircleBounds2D : public Bounds2D<CircleBounds2D>
	{
		glm::vec2 Center;
		F32 Radius;
		CircleBounds2D(const glm::vec2& center = glm::vec2{0.0f}, F32 radius = 1.0f)
			: Center(center), Radius(radius)
		{}
		CircleBounds2D(const CircleBounds2D& first, const CircleBounds2D& second)
		{
			if (CircleContain2D(first, second))
			{
				const CircleBounds2D& biggest = first.Radius > second.Radius ? first : second;
				Center = biggest.Center;
				Radius = biggest.Radius;
				return;
			}
			const glm::vec2 centerOffset = second.Center - first.Center;
			const F32 distance = glm::distance(first.Center, second.Center);
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

	using DefaultBounds2D = AABB2D;

	// Forward declarations.

	class RigidBody2D;
	class BoxCollider2D;
	class CircleCollider2D;
	class EdgeCollider2D;
	class Collider2D;
	struct MassInfo2D;

	// Inspired by.... 

	struct Filter
	{
		U16 CategoryBits = 0x0001;
		// Which categories to collide with.
		U16 MaskBits = 0xFFFF;
		/* 
		Altrenative to category + mask, if for a pair of colliders group is:
			* 0 / different - use category + mask.
			* same and negative - never collide.
			* same and positive - always collide.
		*/
		I32 GroupIndex = 0;
		static bool ShouldCollide(Collider2D* first, Collider2D* second);
	};

	struct ColliderDef2D
	{
		// This collider will be copied.
		Collider2D* Collider = nullptr;
		PhysicsMaterial PhysicsMaterial{};
		Filter Filter{};
		bool IsSensor = false;
		void* UserData = nullptr;
		Collider2D* Clone() const;
	};

	struct ColliderListEntry2D
	{
		Collider2D* Collider = nullptr;
		ColliderListEntry2D* Next = nullptr;
		ColliderListEntry2D* Prev = nullptr;
	};

	class Collider2D
	{
	public:
		enum class Type
		{
			Box = 0, Circle = 1, Edge, TypesCount
		};
		Collider2D(Type type) : m_Type(type) {}
		virtual ~Collider2D() = default;
		Type GetType() const { return m_Type; }
		I32 GetTypeInt() const { return static_cast<I32>(m_Type); }

		void SetUserData(void* userData) { m_UserData = userData; }
		void* GetUserData() const { return m_UserData; }

		void SetAttachedRigidBody(RigidBody2D* rbody) { m_AttachedRigidBody = rbody; }
		const RigidBody2D* GetAttachedRigidBody() const { return m_AttachedRigidBody; }

		void SetPhysicsMaterial(const PhysicsMaterial& material) { m_PhysicsMaterial = material; }
		const PhysicsMaterial& GetPhysicsMaterial() const { return m_PhysicsMaterial; }

		bool IsSensor() const { return m_IsSensor; }
		void SetSensor(bool isSensor) { m_IsSensor = isSensor; }

		const Filter& GetFilter() const { return m_Filter; }
		void SetFilter(const Filter& filter) { m_Filter = filter; }

		static void Destroy(Collider2D* collider);

		virtual Collider2D* Clone() = 0;
		
		virtual DefaultBounds2D GenerateBounds(const Transform2D& transform = Transform2D()) const = 0;
		virtual MassInfo2D CalculateMass() const = 0;

	protected:
		Type m_Type;
		void* m_UserData = nullptr;
		Filter m_Filter;
		PhysicsMaterial m_PhysicsMaterial;
		bool m_IsSensor = false;
		RigidBody2D* m_AttachedRigidBody = nullptr;
	};

	class BoxCollider2D : public Collider2D
	{
	public:
		BoxCollider2D(const glm::vec2& center = glm::vec2{ 0.0f }, const glm::vec2& halfSize = glm::vec2{ 1.0f });
		Collider2D* Clone() override;
		DefaultBounds2D GenerateBounds(const Transform2D& transform = Transform2D()) const override;
		MassInfo2D CalculateMass() const override;

		glm::vec2 GetFaceDirection(I32 vertexId) const;
		glm::vec2 GetVertex(I32 vertexId) const;
		glm::vec2 HalfSize;
		// Center is relative to it's rigidbody.
		glm::vec2 Center;
		// Synthetic thickness for stability.
		static constexpr F32 Radius = 2.0f * 0.005f; //TODO: magic constants.

	};
	
	class CircleCollider2D : public Collider2D
	{
	public:
		CircleCollider2D(const glm::vec2& center = glm::vec2{ 0.0f }, F32 radius = 1.0f);
		Collider2D* Clone() override;
		DefaultBounds2D GenerateBounds(const Transform2D& transform = Transform2D()) const override;
		MassInfo2D CalculateMass() const override;

		F32 Radius;
		// Center is relative to it's rigidbody.
		glm::vec2 Center;
	};

	// Line segment.
	class EdgeCollider2D : public Collider2D
	{
	public:
		EdgeCollider2D(const glm::vec2& start = glm::vec2{ -1.0f, 0.0f }, const glm::vec2& end = glm::vec2{ 1.0f, 0.0f });
		Collider2D* Clone() override;
		DefaultBounds2D GenerateBounds(const Transform2D& transform = Transform2D()) const override;
		MassInfo2D CalculateMass() const override;

		// Normal is computed when needed, as an outward normal from `Start` to `End`.
		// Relative to rigidbody.
		glm::vec2 Start;
		glm::vec2 End;
	};
}
