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
		return AABB2D{ glm::vec3((max + min) * 0.5f, Center.z), (max - min) * 0.5f };
	}

	Collider2D* CircleCollider2D::Clone()
	{
		return New<CircleCollider2D>(Center, Radius);
	}

	DefaultBounds2D CircleCollider2D::GenerateBounds(const Transform2D& transform) const
	{
		return AABB2D{ glm::vec3(transform.Transform(Center), Center.z), {Radius, Radius} };
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
		return AABB2D{ glm::vec3((max + min) * 0.5f, End.z), (max - min) * 0.5f };
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
