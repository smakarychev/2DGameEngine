#pragma once

#include "Bounds2D.h"
#include "Engine/Physics/NewRBE/Newest/PhysicsMaterial.h"
#include "Engine/Physics/NewRBE/Newest/Collision/CollisionFilter.h"
#include "Engine/Physics/NewRBE/Newest/DynamicsData.h"

namespace Engine::WIP::Physics::Newest
{
	class RigidBody2D;
	class Collider2D;

	enum class Collider2DType
	{
		Polygon = 0, Circle, Edge,
		Compound,
		TypesCount
	};

	struct ColliderDesc2D
    {
		friend class PhysicsFactory;
		friend class Collider2D;
		explicit ColliderDesc2D(Collider2DType type) : m_ColliderType(type) {}
        PhysicsMaterial PhysicsMaterial{};
        CollisionFilter CollisionFilter{};
        bool IsSensor{false};
	private:
		Collider2DType m_ColliderType{};
    };
	
    class Collider2D
    {
    	friend class PhysicsFactory;
    public:
	    Collider2D(const ColliderDesc2D& colDesc)
    		: m_Type(colDesc.m_ColliderType), m_Filter(colDesc.CollisionFilter),
    		m_PhysicsMaterial(colDesc.PhysicsMaterial), m_IsSensor(colDesc.IsSensor) {}
		virtual ~Collider2D() = default;

		Collider2DType GetType() const { return m_Type; }
		I32 GetTypeInt() const { return static_cast<I32>(m_Type); }

		void SetPhysicsMaterial(const PhysicsMaterial& material) { m_PhysicsMaterial = material; }
		const PhysicsMaterial& GetPhysicsMaterial() const { return m_PhysicsMaterial; }

    	RigidBody2D* GetRigidBody() const { return m_RigidBody; }
    	void SetRigidBody(RigidBody2D* rigidBody) { m_RigidBody = rigidBody; }

		bool IsSensor() const { return m_IsSensor; }
		void SetSensor(bool isSensor) { m_IsSensor = isSensor; }

		const CollisionFilter& GetCollisionFilter() const { return m_Filter; }
		void SetCollisionFilter(const CollisionFilter& filter) { m_Filter = filter; }

		virtual AABB2D GenerateBounds(const Transform2D& transform) const = 0;
    	virtual AABB2D GenerateLocalBounds() const = 0;
		virtual MassInfo2D CalculateMass() const = 0;
		virtual glm::vec2 GetCenterOfMass() const = 0;
	protected:
		Collider2DType m_Type;
		CollisionFilter m_Filter{};
		PhysicsMaterial m_PhysicsMaterial{};
    	RigidBody2D* m_RigidBody{nullptr};
		bool m_IsSensor{false};
    };
}


