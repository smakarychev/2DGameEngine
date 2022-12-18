#include "enginepch.h"

#include "Collider.h"

#include "Engine/Physics/RigidBodyEngine/RigidBody.h"

namespace Engine::Physics
{
	bool Filter::ShouldCollide(Collider2D* first, Collider2D* second)
	{
		const Filter& filterA = first->GetFilter();
		const Filter& filterB = second->GetFilter();
		if (filterA.GroupIndex == 0 || filterA.GroupIndex != filterB.GroupIndex)
		{
			// Use mask + category.
			return (filterA.CategoryBits & filterB.MaskBits) && (filterB.CategoryBits & filterA.MaskBits);
		}
		else
		{
			if (filterA.GroupIndex > 0)
			{
				return true;
			}
			return false;
		}
	}

	Collider2D* ColliderDef2D::Clone() const
	{
		Collider2D* clone = Collider->Clone();
		clone->SetPhysicsMaterial(PhysicsMaterial);
		clone->SetFilter(Filter);
		clone->SetSensor(IsSensor);
		clone->SetUserData(UserData);
		return clone;
	}

	BoxCollider2D::BoxCollider2D(const glm::vec2& center, const glm::vec2& halfSize)
		: Collider2D(Type::Box),
		HalfSize(halfSize), Center(center)
	{}

	CircleCollider2D::CircleCollider2D(const glm::vec2& center, F32 radius)
		: Collider2D(Type::Circle),
		Radius(radius), Center(center)
	{}

	EdgeCollider2D::EdgeCollider2D(const glm::vec2& start, const glm::vec2& end)
		: Collider2D(Type::Edge),
		Start(start), End(end)
	{}

	glm::vec2 BoxCollider2D::GetFaceDirection(I32 vertexId) const
	{
		static constexpr std::array<glm::vec2, 4> localDirs {
			glm::vec2{ -1.0f,  0.0f },
			glm::vec2{  0.0f, -1.0f },
			glm::vec2{  1.0f,  0.0f },
			glm::vec2{  0.0f,  1.0f }
		};
		return m_AttachedRigidBody->GetTransform().TransformDirection(localDirs[vertexId]);
	}

	glm::vec2 BoxCollider2D::GetVertex(I32 vertexId) const
	{
		switch (vertexId)
		{
		case 0: return m_AttachedRigidBody->GetTransform().Transform({ Center.x - HalfSize.x, Center.y - HalfSize.y });
		case 1: return m_AttachedRigidBody->GetTransform().Transform({ Center.x + HalfSize.x, Center.y - HalfSize.y });
		case 2: return m_AttachedRigidBody->GetTransform().Transform({ Center.x + HalfSize.x, Center.y + HalfSize.y });
		case 3: return m_AttachedRigidBody->GetTransform().Transform({ Center.x - HalfSize.x, Center.y + HalfSize.y });
		default: ENGINE_CORE_FATAL("Impossible vertex index!"); return { 0.0f, 0.0f };
		}
	}

	Collider2D* BoxCollider2D::Clone()
	{
		return New<BoxCollider2D>(Center, HalfSize);
	}
	
	DefaultBounds2D BoxCollider2D::GenerateBounds(const Transform2D& transform) const
	{
		glm::vec2 vertices[4]{
			{Center.x - HalfSize.x, Center.y - HalfSize.y},
			{Center.x + HalfSize.x, Center.y - HalfSize.y},
			{Center.x + HalfSize.x, Center.y + HalfSize.y},
			{Center.x - HalfSize.x, Center.y + HalfSize.y},
		};
		glm::vec2 max{ -std::numeric_limits<F32>::max(), -std::numeric_limits<F32>::max() };
		glm::vec2 min{  std::numeric_limits<F32>::max(),  std::numeric_limits<F32>::max() };
		for (U32 i = 0; i < 4; i++)
		{
			glm::vec2 transformed = transform.Transform(vertices[i]);
			max = glm::vec2{ Math::Max(max.x, transformed.x), Math::Max(max.y, transformed.y) };
			min = glm::vec2{ Math::Min(min.x, transformed.x), Math::Min(min.y, transformed.y) };
		}
		return AABB2D{ (max + min) * 0.5f, (max - min) * 0.5f };
	}

	MassInfo2D BoxCollider2D::CalculateMass() const
	{
		static constexpr auto inertiaCoeff = 4.0f / 12.0f;
		F32 mass = HalfSize.x * HalfSize.y * 4.0f * m_PhysicsMaterial.Density;
		F32 inertia = mass * inertiaCoeff * glm::length2(HalfSize);
		return MassInfo2D{ .Mass = mass, .Inertia = inertia, .CenterOfMass = Center };
	}

	Collider2D* CircleCollider2D::Clone()
	{
		return New<CircleCollider2D>(Center, Radius);
	}

	DefaultBounds2D CircleCollider2D::GenerateBounds(const Transform2D& transform) const
	{
		return AABB2D{ transform.Transform(Center), {Radius, Radius} };
	}

	MassInfo2D CircleCollider2D::CalculateMass() const
	{
		static constexpr auto inertiaCoeff = 1.0f / 2.0f;
		F32 mass = Math::Pi() * Radius * Radius *m_PhysicsMaterial.Density;
		F32 inertia = mass * inertiaCoeff * Radius * Radius;
		return MassInfo2D{ .Mass = mass, .Inertia = inertia, .CenterOfMass = Center };
	}

	Collider2D* EdgeCollider2D::Clone()
	{
		return New<EdgeCollider2D>(Start, End);
	}

	DefaultBounds2D EdgeCollider2D::GenerateBounds(const Transform2D& transform) const
	{
		glm::vec2 worldStart = transform.Transform(Start);
		glm::vec2 worldEnd = transform.Transform(End);
		glm::vec2 max{
			Math::Max(worldEnd.x, worldStart.x),
			Math::Max(worldEnd.y, worldStart.y)
		};
		glm::vec2 min{
			Math::Min(worldEnd.x, worldStart.x),
			Math::Min(worldEnd.y, worldStart.y)
		};
		return AABB2D{ (max + min) * 0.5f, (max - min) * 0.5f };
	}

	MassInfo2D EdgeCollider2D::CalculateMass() const
	{
		ENGINE_CORE_ASSERT(false, "Not yet implemented")
		F32 mass = 0.0f;
		F32 inertia = 0.0f;
		return MassInfo2D{ .Mass = mass, .Inertia = inertia, .CenterOfMass = glm::vec2{0.0f} };
	}

	void Collider2D::Destroy(Collider2D* collider)
	{
		switch (collider->m_Type)
		{
		case Type::Box:
		{
			Delete<BoxCollider2D>(reinterpret_cast<BoxCollider2D*>(collider));
			break;
		}
		case Type::Circle:
		{
			Delete<CircleCollider2D>(reinterpret_cast<CircleCollider2D*>(collider));
			break;
		}
		case Type::Edge:
		{
			Delete<EdgeCollider2D>(reinterpret_cast<EdgeCollider2D*>(collider));
			break;
		}
		case Type::TypesCount: 
		ENGINE_CORE_FATAL("Fatal error."); break;
		}
	}
}