#include "enginepch.h"
#include "Contacts.h"

namespace Engine
{
	BoxBoxContact2D::BoxBoxContact2D(BoxCollider2D* first, BoxCollider2D* second)
		: m_First(first), m_Second(second)
	{}

	Contact2D* BoxBoxContact2D::Create(Collider2D* a, Collider2D* b)
	{
		return New<BoxBoxContact2D>(
			static_cast<BoxCollider2D*>(a),
			static_cast<BoxCollider2D*>(b)
		);
	}

	void BoxBoxContact2D::Destroy(Contact2D* contact)
	{
		Delete<BoxBoxContact2D>(
			reinterpret_cast<BoxBoxContact2D*>(contact)
		);
	}

	U32 BoxBoxContact2D::GenerateContacts(std::vector<ContactInfo2D>& contacts)
	{
		// TODO: implement.
		return 0;
	}
	
	CircleCircleContact2D::CircleCircleContact2D(CircleCollider2D* first, CircleCollider2D* second)
		: m_First(first), m_Second(second)
	{
	}

	Contact2D* CircleCircleContact2D::Create(Collider2D* a, Collider2D* b)
	{
		return New<CircleCircleContact2D>(
			static_cast<CircleCollider2D*>(a),
			static_cast<CircleCollider2D*>(b)
		);
	}

	void CircleCircleContact2D::Destroy(Contact2D* contact)
	{
		Delete<CircleCircleContact2D>(
			reinterpret_cast<CircleCircleContact2D*>(contact)
		);
	}

	U32 CircleCircleContact2D::GenerateContacts(std::vector<ContactInfo2D>& contacts)
	{
		// Check if there is a collision.
		if (m_First->Center.z != m_Second->Center.z) return 0;

		glm::vec2 distVec = glm::vec2{ m_First->Center.x - m_Second->Center.x,
			m_First->Center.y - m_Second->Center.y };
		F32 minDist = m_First->Radius + m_Second->Radius;
		F32 minDistSquared = minDist * minDist;
		F32 distSquared = glm::length2(distVec);
		if (distSquared > minDistSquared) return 0;
		
		// Else we have a collision.
		F32 dist = Math::Sqrt(distSquared);
		glm::vec2 normal = distVec / dist;
		ContactInfo2D contactInfo;
		contactInfo.Normal = normal;
		contactInfo.Point = glm::vec2(m_First->Center) + distVec * 0.5f;
		contactInfo.PenetrationDepth = minDist - dist;
		
		contacts.push_back(contactInfo);

		return 1;
	}

	BoxCircleContact2D::BoxCircleContact2D(BoxCollider2D* box, CircleCollider2D* circle)
		: m_Box(box), m_Circle(circle)
	{
	}

	Contact2D* BoxCircleContact2D::Create(Collider2D* a, Collider2D* b)
	{
		return New<BoxCircleContact2D>(
			static_cast<BoxCollider2D*>(a),
			static_cast<CircleCollider2D*>(b)
		);
	}

	void BoxCircleContact2D::Destroy(Contact2D* contact)
	{
		Delete<BoxCircleContact2D>(
			reinterpret_cast<BoxCircleContact2D*>(contact)
		);
	}

	U32 BoxCircleContact2D::GenerateContacts(std::vector<ContactInfo2D>& contacts)
	{
		// TODO: implement.
		return 0;
	}

	EdgeCircleContact2D::EdgeCircleContact2D(EdgeCollider2D* edge, CircleCollider2D* circle)
		: m_Edge(edge), m_Circle(circle)
	{
	}
	
	Contact2D* EdgeCircleContact2D::Create(Collider2D* a, Collider2D* b)
	{
		return New<EdgeCircleContact2D>(
			static_cast<EdgeCollider2D*>(a),
			static_cast<CircleCollider2D*>(b)
		);
	}
	
	void EdgeCircleContact2D::Destroy(Contact2D* contact)
	{
		Delete<EdgeCircleContact2D>(
			reinterpret_cast<EdgeCircleContact2D*>(contact)
		);
	}
	
	U32 EdgeCircleContact2D::GenerateContacts(std::vector<ContactInfo2D>& contacts)
	{
		if (m_Edge->End.z != m_Circle->Center.z) return 0;
		
		// Compute edge normal.
		glm::vec2 edgeDir = m_Edge->End - m_Edge->Start;
		glm::vec2 normal = glm::vec2{ edgeDir.y, -edgeDir.x };
		normal = glm::normalize(normal);
		F32 offset = -glm::dot(normal, glm::vec2(m_Edge->Start));

		F32 distanceToPlane = glm::dot(normal, glm::vec2(m_Circle->Center)) - offset;
		// Check if there is a collision.
		if (distanceToPlane * distanceToPlane > m_Circle->Radius * m_Circle->Radius)
		{
			return 0;
		}

		ContactInfo2D contact;
		contact.Point = glm::vec2(m_Circle->Center) - normal * distanceToPlane;
		
		F32 depth = -distanceToPlane;
		// If circle is on other side of the edge.
		if (distanceToPlane < 0)
		{
			normal = -normal;
			depth = -depth;
		}
		depth += m_Circle->Radius;

		contact.Normal = normal;
		contact.PenetrationDepth = depth;
		contacts.push_back(contact);

		return 1;
	}
	
	EdgeBoxContact2D::EdgeBoxContact2D(EdgeCollider2D* edge, BoxCollider2D* box)
		: m_Edge(edge), m_Box(box)
	{
	}
	
	Contact2D* EdgeBoxContact2D::Create(Collider2D* a, Collider2D* b)
	{
		return New<EdgeBoxContact2D>(
			static_cast<EdgeCollider2D*>(a),
			static_cast<BoxCollider2D*>(b)
		);
	}
	
	void EdgeBoxContact2D::Destroy(Contact2D* contact)
	{
		Delete<EdgeBoxContact2D>(
			reinterpret_cast<EdgeBoxContact2D*>(contact)
		);
	}
	
	U32 EdgeBoxContact2D::GenerateContacts(std::vector<ContactInfo2D>& contacts)
	{
		if (!BoxHalfSpaceCollision2D(*m_Box, *m_Edge)) return 0;

		// Check every vertex against edge.
		glm::vec2 vertices[4] = {
			glm::vec2(m_Box->Center) + glm::vec2{-m_Box->HalfSize.x, -m_Box->HalfSize.y},
			glm::vec2(m_Box->Center) + glm::vec2{ m_Box->HalfSize.x, -m_Box->HalfSize.y},
			glm::vec2(m_Box->Center) + glm::vec2{ m_Box->HalfSize.x,  m_Box->HalfSize.y},
			glm::vec2(m_Box->Center) + glm::vec2{-m_Box->HalfSize.x,  m_Box->HalfSize.y}
		};
		
		glm::vec2 edgeDir = m_Edge->End - m_Edge->Start;
		glm::vec2 normal = glm::vec2{ edgeDir.y, -edgeDir.x };
		normal = glm::normalize(normal);
		F32 offset = -glm::dot(normal, glm::vec2(m_Edge->Start));

		U32 foundContacts = 0;
		for (U32 i = 0; i < 4; i++)
		{
			F32 vertexDistance = glm::dot(vertices[i], normal);
			if (vertexDistance <= offset)
			{
				// Create contact data.
				ContactInfo2D contact;
				contact.Normal = normal;
				contact.Point = normal * (vertexDistance - offset) + vertices[i];
				contact.PenetrationDepth = offset - vertexDistance;
				
				contacts.push_back(contact);
				foundContacts++;
			}
		}

		return foundContacts;
	}

	ContactRegistation ContactManager::s_Registry
		[static_cast<U32>(Collider2D::Type::TypesCount)]
		[static_cast<U32>(Collider2D::Type::TypesCount)];

	Contact2D* ContactManager::Create(Collider2D* a, Collider2D* b)
	{
		U32 aTypeInt = a->GetTypeInt();
		U32 bTypeInt = b->GetTypeInt();
		if (s_Registry[aTypeInt][bTypeInt].IsPrimary)
		{
			return s_Registry[aTypeInt][bTypeInt].CreateFn(a, b);
		}
		else
		{
			return s_Registry[aTypeInt][bTypeInt].CreateFn(b, a);
		}
	}

	// Create matrix of dipatch functions.
	void ContactManager::Init()
	{
		AddRegistration(BoxBoxContact2D::Create, BoxBoxContact2D::Destroy, Collider2D::Type::Box, Collider2D::Type::Box);
		AddRegistration(BoxCircleContact2D::Create, BoxCircleContact2D::Destroy, Collider2D::Type::Box, Collider2D::Type::Circle);
		AddRegistration(CircleCircleContact2D::Create, CircleCircleContact2D::Destroy, Collider2D::Type::Circle, Collider2D::Type::Circle);
		AddRegistration(EdgeCircleContact2D::Create, EdgeCircleContact2D::Destroy, Collider2D::Type::Edge, Collider2D::Type::Circle);
	}

	void ContactManager::AddRegistration(
		ContactRegistation::OnCreateFn createFn,
		ContactRegistation::OnDestroyFn destroyFn,
		Collider2D::Type typeA,
		Collider2D::Type typeB
	)
	{
		U32 typeAI = static_cast<U32>(typeA);
		U32 typeBI = static_cast<U32>(typeB);
		s_Registry[typeAI][typeBI].CreateFn = createFn;
		s_Registry[typeAI][typeBI].DestroyFn = destroyFn;
		s_Registry[typeAI][typeBI].IsPrimary = true;

		if (typeAI != typeBI)
		{
			s_Registry[typeBI][typeAI].CreateFn = createFn;
			s_Registry[typeBI][typeAI].DestroyFn = destroyFn;
			s_Registry[typeBI][typeAI].IsPrimary = false;
		}
	}

}


