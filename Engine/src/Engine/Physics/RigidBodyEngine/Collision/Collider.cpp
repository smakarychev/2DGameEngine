#include "enginepch.h"

#include "Collider.h"

namespace Engine
{
	BoxCollider2D::BoxCollider2D(const glm::vec3& center, const glm::vec2& halfSize)
		: Collider2D(Type::Box),
		HalfSize(halfSize), Center(center)
	{}

	CircleCollider2D::CircleCollider2D(const glm::vec3& center, F32 radius)
		: Collider2D(Type::Circle),
		Radius(radius), Center(center)
	{}

	EdgeCollider2D::EdgeCollider2D(const glm::vec3& start, const glm::vec3& end)
		: Collider2D(Type::Edge),
		Start(start), End(end)
	{
		ENGINE_CORE_ASSERT(Start.z == End.z, "Edge must have same z coordinate.");
	}

	Collider2D* BoxCollider2D::Clone()
	{
		return New<BoxCollider2D>(Center, HalfSize);
	}

	Collider2D* CircleCollider2D::Clone()
	{
		return New<CircleCollider2D>(Center, Radius);
	}

	Collider2D* EdgeCollider2D::Clone()
	{
		return New<EdgeCollider2D>(Start, End);
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
		}
	}

}
