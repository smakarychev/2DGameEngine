#include "enginepch.h"

#include "Collider.h"

#include "Engine/ECS/Components.h"
#include "Engine/Math/LinearAlgebra.h"
#include "Engine/Physics/NewRBE/RigidBody.h"

namespace Engine::WIP::Physics
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

	Collider2D* ColliderDef2D::Clone(MemoryManager::ManagedPoolAllocator& allocator) const
	{
		Collider2D* clone = Collider->Clone(allocator);
		clone->SetPhysicsMaterial(PhysicsMaterial);
		clone->SetFilter(Filter);
		clone->SetSensor(IsSensor);
		clone->SetUserData(UserData);
		return clone;
	}

	CircleCollider2D::CircleCollider2D(const glm::vec2& center, F32 radius)
		: Collider2D(Type::Circle),
		Radius(radius), Center(center)
	{}

	EdgeCollider2D::EdgeCollider2D(const glm::vec2& start, const glm::vec2& end)
		: Collider2D(Type::Edge),
		Start(start), End(end)
	{}

	Collider2D* CircleCollider2D::Clone(MemoryManager::ManagedPoolAllocator& allocator)
	{
		return NewAlloc<CircleCollider2D>(allocator, Center, Radius);
	}

	DefaultBounds2D CircleCollider2D::GenerateBounds(const Transform2D& transform) const
	{
		return AABB2D{ transform.Transform(Center), {Radius, Radius} };
	}

	MassInfo2D CircleCollider2D::CalculateMass() const
	{
		static constexpr auto inertiaCoeff = 1.0f / 2.0f;
		F32 mass = Math::Pi() * Radius * Radius * m_PhysicsMaterial.Density;
		F32 inertia = mass * inertiaCoeff * Radius * Radius;
		return MassInfo2D{ .Mass = mass, .Inertia = inertia, .CenterOfMass = Center };
	}

	glm::vec2 CircleCollider2D::GetCenterOfMass() const
	{
		return Center;
	}

	Collider2D* EdgeCollider2D::Clone(MemoryManager::ManagedPoolAllocator& allocator)
	{
		return NewAlloc<EdgeCollider2D>(allocator, Start, End);
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

	glm::vec2 EdgeCollider2D::GetCenterOfMass() const
	{
		return (End + Start) * 0.5f;
	}

	PolygonCollider2D::PolygonCollider2D()
		: Collider2D(Type::Polygon)
	{
	}

	PolygonCollider2D::PolygonCollider2D(const std::vector<glm::vec2>& vertices)
		: Collider2D(Type::Polygon)
	{
		BuildPolygonFromPoints(vertices);
	}

	PolygonCollider2D::PolygonCollider2D(const std::vector<glm::vec2>& vertices, const std::vector<glm::vec2>& normals, const glm::vec2& center)
		: Collider2D(Type::Polygon), m_Vertices(vertices), m_Normals(normals), m_Center(center)
	{
		// Trust no one.
		ENGINE_CORE_ASSERT(vertices.size() == normals.size(), "Invalid parameters: vertices count is not same with normals count.")
	}

	void PolygonCollider2D::BuildPolygonFromPoints(const std::vector<glm::vec2>& vertices)
	{
		if (vertices.size() < 3) { SetAsErrorBox(); return; }
		// Leave only unique vertices.
		static constexpr F32 distSquaredTolerance = m_Radius * m_Radius * 0.5f * 0.5f;
		std::vector<glm::vec2> actualVertices;
		actualVertices.reserve(vertices.size());
		for (auto& v : vertices)
		{
			auto it = std::ranges::find_if(actualVertices, [&v](const auto& av) { return glm::distance2(v, av) < distSquaredTolerance; });
			if (it == actualVertices.end()) actualVertices.push_back(v);
		}
		if (actualVertices.size() < 3) { SetAsErrorBox(); return; }
		m_Vertices = ConvexHull(vertices);
		if (m_Vertices.empty()) { SetAsErrorBox(); return; }
		// Calculate normals.
		m_Normals.reserve(m_Vertices.size());
		for (U32 vI = 0; vI < m_Vertices.size(); vI++)
		{
			U32 startI = vI;
			U32 endI = vI != m_Vertices.size() - 1 ? vI + 1 : 0;
			glm::vec2 dir = m_Vertices[endI] - m_Vertices[startI];
			m_Normals.push_back(glm::normalize(Math::Cross2D(dir, 1.0f)));
		}
		// Finally calculate center point.
		m_Center = ComputeCentroid();
	}

	void PolygonCollider2D::SetAsErrorBox()
	{
		ENGINE_CORE_ERROR("Cannot create convex polygon from points, creating default box instead.");
		SetAsBox(glm::vec2{0.0f, 0.0f}, glm::vec2{0.5f, 0.5f});
	}

	glm::vec2 PolygonCollider2D::ComputeCentroid()
	{
		if (m_Vertices.size() < 3) { ENGINE_CORE_ERROR("Cannot compute centroid of degenerate polygon."); return {}; }
		
		glm::vec2 centroid{0.0f};
		F32 area = 0.0f;
		glm::vec2 refPoint = m_Vertices.front();
		constexpr F32 oneThird = 1.0f / 3.0f; 

		for (U32 i = 1; i < m_Vertices.size() - 1; i++)
		{
			glm::vec2 tP1 = m_Vertices[i] - refPoint;
			glm::vec2 tP2 = m_Vertices[i + 1] - refPoint;
			F32 D = Math::Cross2D(tP1, tP2);
			F32 tArea = D * 0.5f;
			area += tArea;
			centroid += tArea * oneThird * (tP1 + tP2);
		}
		centroid = (1.0f / area) * centroid + refPoint;
		return centroid;
	}

	PolygonCollider2D::Box2D PolygonCollider2D::SetAsBox(const glm::vec2& halfSize, const glm::vec2& center, const Rotation& rotation)
	{
		m_Center = center;
		m_Vertices = {
			{center - halfSize},
			{center.x + halfSize.x, center.y - halfSize.y},
			{center + halfSize},
			{center.x - halfSize.x, center.y + halfSize.y},
		};
		m_Normals = {
			{ 0.0f, -1.0f},
			{ 1.0f,  0.0f},
			{ 0.0f,  1.0f},
			{-1.0f,  0.0f},	
		};
		for (auto& v : m_Vertices) v = TransformDir2D(v, rotation);
		for (auto& n : m_Normals) n = TransformDir2D(n, rotation);
		return Box2D{.Center = center, .HalfSize = halfSize};
	}

	PolygonCollider2D::Box2D PolygonCollider2D::GetAsBox()
	{
		glm::vec2 min{std::numeric_limits<F32>::max()}, max{-std::numeric_limits<F32>::max()};
		for (auto& v : m_Vertices)
		{
			min = Math::Min(v, min);
			max = Math::Max(v, max);
		}
		AABB2D bounds = DefaultBounds2D::GetFromMinMax(min, max);
		return Box2D{.Center = bounds.Center, .HalfSize = bounds.HalfSize};
	}

	Collider2D* PolygonCollider2D::Clone(MemoryManager::ManagedPoolAllocator& allocator)
	{
		return NewAlloc<PolygonCollider2D>(allocator, m_Vertices, m_Normals, m_Center);
	}

	DefaultBounds2D PolygonCollider2D::GenerateBounds(const Transform2D& transform) const
	{
		glm::vec2 min{std::numeric_limits<F32>::max()}, max{-std::numeric_limits<F32>::max()};
		for (auto& v : m_Vertices)
		{
			auto transformed = GetAttachedTransform().Transform(v);
			min = Math::Min(transformed, min);
			max = Math::Max(transformed, max);
		}
		return DefaultBounds2D::GetFromMinMax(min, max);
	}

	MassInfo2D PolygonCollider2D::CalculateMass() const
	{
		if (m_Vertices.size() < 3) { ENGINE_CORE_ERROR("Cannot compute mass of degenerate polygon."); return {}; }
		
		glm::vec2 centroid{0.0f};
		F32 area = 0.0f;
		F32 inertia = 0.0f;
		glm::vec2 refPoint = m_Vertices.front();
		constexpr F32 oneThird = 1.0f / 3.0f;
		
		for (U32 i = 1; i < m_Vertices.size() - 1; i++)
		{
			glm::vec2 tP1 = m_Vertices[i] - refPoint;
			glm::vec2 tP2 = m_Vertices[i + 1] - refPoint;
			F32 D = Math::Cross2D(tP1, tP2);
			F32 tArea = D * 0.5f;
			area += tArea;
			centroid += tArea * oneThird * (tP1 + tP2);

			F32 integralX2 = tP1.x * tP1.x + tP1.x * tP2.x + tP2.x * tP2.x;
			F32 integralY2 = tP1.y * tP1.y + tP1.y * tP2.y + tP2.y * tP2.y;

			inertia += (0.25f * oneThird * D) * (integralX2 + integralY2);
		}
		MassInfo2D massInfo{};
		massInfo.Mass = area * m_PhysicsMaterial.Density;
		centroid = (1.0f / area) * centroid;
		massInfo.CenterOfMass = centroid + refPoint;
		massInfo.Inertia = inertia * m_PhysicsMaterial.Density;
		massInfo.Inertia += massInfo.Mass * (glm::dot(massInfo.CenterOfMass, massInfo.CenterOfMass) - glm::dot(centroid, centroid));
		
		return massInfo;
	}

	glm::vec2 PolygonCollider2D::GetCenterOfMass() const
	{
		return m_Center;
	}

	const Transform2D& Collider2D::GetAttachedTransform() const { return m_AttachedRigidBody->GetTransform(); }

	Transform2D& Collider2D::GetAttachedTransform() { return m_AttachedRigidBody->GetTransform(); }

	void Collider2D::Destroy(MemoryManager::ManagedPoolAllocator& allocator, Collider2D* collider)
	{
		switch (collider->m_Type)
		{
		case Type::Polygon:
		{
			DeleteAlloc<PolygonCollider2D>(allocator, reinterpret_cast<PolygonCollider2D*>(collider));
			break;
		}
		case Type::Circle:
		{
			DeleteAlloc<CircleCollider2D>(allocator, reinterpret_cast<CircleCollider2D*>(collider));
			break;
		}
		case Type::Edge:
		{
			DeleteAlloc<EdgeCollider2D>(allocator, reinterpret_cast<EdgeCollider2D*>(collider));
			break;
		}
		case Type::TypesCount: 
		ENGINE_CORE_FATAL("Fatal error."); break;
		}
	}
}
