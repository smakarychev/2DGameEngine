#pragma once

#include "Bounds2D.h"
#include "Engine/Physics/NewRBE/Newest/PhysicsMaterial.h"
#include "Engine/Physics/NewRBE/Newest/CollisionFilter.h"
#include "Engine/Physics/NewRBE/Newest/DynamicsData.h"

namespace Engine::WIP::Physics::Newest
{
    class Collider2D;

    struct ColliderDesc2D
    {
        // This collider will be copied.
        Collider2D* Collider{nullptr};
        PhysicsMaterial PhysicsMaterial{};
        CollisionFilter CollisionFilter{};
        void* UserData{nullptr};
        bool IsSensor{false};
    };

	enum class Collider2DType
	{
		Polygon = 0, Circle, Edge,
		Compound,
		TypesCount
	};

    class Collider2D
    {
    public:
	    Collider2D(Collider2DType type) : m_Type(type) {}

		virtual ~Collider2D() = default;
		Collider2DType GetType() const { return m_Type; }
		I32 GetTypeInt() const { return static_cast<I32>(m_Type); }

		void SetPhysicsMaterial(const PhysicsMaterial& material) { m_PhysicsMaterial = material; }
		const PhysicsMaterial& GetPhysicsMaterial() const { return m_PhysicsMaterial; }

		bool IsSensor() const { return m_IsSensor; }
		void SetSensor(bool isSensor) { m_IsSensor = isSensor; }

		const CollisionFilter& GetFilter() const { return m_Filter; }
		void SetFilter(const CollisionFilter& filter) { m_Filter = filter; }

		virtual DefaultBounds2D GenerateBounds(const Transform2D& transform) const = 0;
		virtual MassInfo2D CalculateMass() const = 0;
		virtual glm::vec2 GetCenterOfMass() const = 0;
		
	protected:
		Collider2DType m_Type;
		CollisionFilter m_Filter;
		PhysicsMaterial m_PhysicsMaterial;
		bool m_IsSensor = false;
    };
}


