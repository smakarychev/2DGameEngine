#include "enginepch.h"

#include "Contacts.h"

#include "Engine/Math/LinearAlgebra.h"

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

	U32 BoxBoxContact2D::GenerateContacts(std::vector<ContactManifold2D>& manifolds)
	{
		// SAT.
		SATQuery satA = SATFaceDirections(*m_First, *m_Second, 
			m_First->GetAttachedRigidBody()->GetTransform(), m_Second->GetAttachedRigidBody()->GetTransform());
		if (satA.Distance > 0.0f) return 0;

		SATQuery satB = SATFaceDirections(*m_Second, *m_First,
			m_Second->GetAttachedRigidBody()->GetTransform(), m_First->GetAttachedRigidBody()->GetTransform());
		if (satB.Distance > 0.0f) return 0;
		
		// We need the axis with smallest depth.
		SATQuery sat;
		BoxCollider2D* primary = nullptr;
		BoxCollider2D* secondary = nullptr;
		// Add some bias to satA for coherency.
		static constexpr auto satBias = 0.001f;
		if (satA.Distance > satB.Distance + satBias)
		{
			sat = satA;
			primary = m_First;
			secondary = m_Second;
		}
		else
		{
			sat = satB;
			primary = m_Second;
			secondary = m_First;
		}
		glm::vec2 refFaceDir = primary->GetFaceDirection(sat.FaceIndex);
		I32 incidentFaceI = FindIncidentFaceIndex(*secondary,
			refFaceDir,
			secondary->GetAttachedRigidBody()->GetTransform());
		glm::vec2 incidentFaceDir = secondary->GetFaceDirection(incidentFaceI);


		// Clip incident face by side planes of reference face.
		// Note that ref face in a normal / anti-normal of side planes.
		//? Move to other place?
		LineSegment2D incidentFace{
			.Start = secondary->GetVertex(incidentFaceI),
			.End = secondary->GetVertex(incidentFaceI < 3 ? incidentFaceI + 1 : 0)
		};
		Line2D sideA{
			.Offset = -glm::dot(primary->GetVertex(sat.FaceIndex), refFaceDir),
			.Normal = refFaceDir
		};
		Line2D sideB{
			.Offset = glm::dot(primary->GetVertex(sat.FaceIndex < 3 ? sat.FaceIndex + 1 : 0), refFaceDir),
			.Normal = -refFaceDir
		};

		ClipLineSegmentToLine(incidentFace, incidentFace, sideA);
		ClipLineSegmentToLine(incidentFace, incidentFace, sideB);

		// Keep all points below reference face.
		std::array<glm::vec2, 2> clippedPoints{ incidentFace.Start, incidentFace.End };
		glm::vec2 refFaceNormal = glm::vec2{ -refFaceDir.y, refFaceDir.x };
		F32 refFaceOffset = -glm::dot(refFaceNormal, primary->GetVertex(sat.FaceIndex));
		ContactManifold2D manifold;
		manifold.Bodies[0] = const_cast<RigidBody2D*>(primary->GetAttachedRigidBody());
		manifold.Bodies[1] = const_cast<RigidBody2D*>(secondary->GetAttachedRigidBody());
		manifold.ContactNormal = -refFaceNormal;
		manifold.ContactCount = 0;
		for (U32 i = 0; i < clippedPoints.size(); i++)
		{
			// Compute distance to reference face.
			F32 distance = glm::dot(clippedPoints[i], refFaceNormal) + refFaceOffset;
			if (distance > 0.0f) continue;
			//TODO: Move contact point onto the ref face (this helps coherence).

			ContactInfo2D contactInfo;
			contactInfo.Point = clippedPoints[i];
			contactInfo.PenetrationDepth = -distance;
			manifold.Contacts[manifold.ContactCount] = contactInfo;
			manifold.ContactCount++;
		}
		manifolds.push_back(manifold);
		return manifold.ContactCount;
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

	U32 CircleCircleContact2D::GenerateContacts(std::vector<ContactManifold2D>& manifolds)
	{
		// Transform positions to world space.
		glm::vec2 firstCenter = m_First->GetAttachedRigidBody()->TransformToWorld(m_First->Center);
		glm::vec2 secondCenter = m_Second->GetAttachedRigidBody()->TransformToWorld(m_Second->Center);

		// Check if there is a collision.
		glm::vec2 distVec = glm::vec2{ firstCenter.x - secondCenter.x,
			firstCenter.y - secondCenter.y };
		F32 minDist = m_First->Radius + m_Second->Radius;
		F32 minDistSquared = minDist * minDist;
		F32 distSquared = glm::length2(distVec);
		if (distSquared > minDistSquared) return 0;
		
		// Else we have a collision.
		F32 dist = Math::Sqrt(distSquared);
		glm::vec2 normal = distVec / dist;
		ContactManifold2D manifold;
		manifold.Bodies[0] = const_cast<RigidBody2D*>(m_First->GetAttachedRigidBody());
		manifold.Bodies[1] = const_cast<RigidBody2D*>(m_Second->GetAttachedRigidBody());
		manifold.ContactNormal = normal;
		manifold.ContactCount = 1;
		ContactInfo2D contactInfo;
		contactInfo.Point = firstCenter + distVec * 0.5f;
		contactInfo.PenetrationDepth = minDist - dist;
		manifold.Contacts[0] = contactInfo;
		
		manifolds.push_back(manifold);
		return manifold.ContactCount;
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

	U32 BoxCircleContact2D::GenerateContacts(std::vector<ContactManifold2D>& manifolds)
	{
		// Tranform circle to box coordinate space.
		glm::vec2 circleCenter = m_Circle->GetAttachedRigidBody()->TransformToWorld(m_Circle->Center);
		glm::vec2 circleCenterRel = m_Box->GetAttachedRigidBody()->
			TransformToLocal(circleCenter);
		
		// Clamp each coordinate to the box.
		// `closestPoint` is a point on box.
		glm::vec2 closestPoint{ 0.0f };
		F32 dist = circleCenterRel.x;
		if (dist >  m_Box->HalfSize.x) dist =  m_Box->HalfSize.x;
		if (dist < -m_Box->HalfSize.x) dist = -m_Box->HalfSize.x;
		closestPoint.x = dist;

		dist = circleCenterRel.y;
		if (dist >  m_Box->HalfSize.y) dist =  m_Box->HalfSize.y;
		if (dist < -m_Box->HalfSize.y) dist = -m_Box->HalfSize.y;
		closestPoint.y = dist;

		F32 minDistanceSquared = m_Circle->Radius * m_Circle->Radius;
		F32 distanceSquared = glm::distance2(closestPoint, circleCenterRel);
		if (distanceSquared > minDistanceSquared) return 0;

		// Else we have intersection.
		// Transform closest point to world space.
		closestPoint = m_Box->GetAttachedRigidBody()->TransformToWorld(closestPoint);

		ContactManifold2D manifold;
		manifold.Bodies[0] = const_cast<RigidBody2D*>(m_Box->GetAttachedRigidBody());
		manifold.Bodies[1] = const_cast<RigidBody2D*>(m_Circle->GetAttachedRigidBody());
		manifold.ContactCount = 1;
		manifold.ContactNormal = closestPoint - circleCenter;
		if (manifold.ContactNormal.x != 0.0 || manifold.ContactNormal.y != 0)
			manifold.ContactNormal = glm::normalize(manifold.ContactNormal);
		
		ContactInfo2D contactInfo;
		contactInfo.Point = closestPoint;
		contactInfo.PenetrationDepth = m_Circle->Radius - Math::Sqrt(distanceSquared);
		manifold.Contacts[0] = contactInfo;

		manifolds.push_back(manifold);
		return manifold.ContactCount;
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
	
	U32 EdgeCircleContact2D::GenerateContacts(std::vector<ContactManifold2D>& manifolds)
	{
		// Transform to world space.
		glm::vec2 edgeStart = m_Edge->GetAttachedRigidBody()->TransformToWorld(m_Edge->Start);
		glm::vec2 edgeEnd = m_Edge->GetAttachedRigidBody()->TransformToWorld(m_Edge->End);
		glm::vec2 circleCenter = m_Circle->GetAttachedRigidBody()->TransformToWorld(m_Circle->Center);
		
		// Compute edge normal.
		glm::vec2 edgeDir = edgeEnd - edgeStart;
		glm::vec2 normal = glm::vec2{ -edgeDir.y, edgeDir.x };
		normal = glm::normalize(normal);
		F32 offset = -glm::dot(normal, glm::vec2(edgeStart));

		F32 distanceToPlane = glm::dot(normal, circleCenter) - offset;
		// Check if there is a collision.
		if (distanceToPlane * distanceToPlane > m_Circle->Radius * m_Circle->Radius)
		{
			return 0;
		}

		ContactInfo2D contactInfo;
		contactInfo.Point = circleCenter - normal * distanceToPlane;
		
		F32 depth = -distanceToPlane;
		// If circle is on other side of the edge.
		if (distanceToPlane < 0)
		{
			normal = -normal;
			depth = -depth;
		}
		depth += m_Circle->Radius;

		contactInfo.PenetrationDepth = depth;

		ContactManifold2D manifold;
		manifold.Bodies[0] = const_cast<RigidBody2D*>(m_Edge->GetAttachedRigidBody());
		manifold.Bodies[1] = const_cast<RigidBody2D*>(m_Circle->GetAttachedRigidBody());
		manifold.ContactNormal = normal;
		manifold.ContactCount = 1;
		manifold.Contacts[0] = contactInfo;

		manifolds.push_back(manifold);
		return manifold.ContactCount;
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
	
	U32 EdgeBoxContact2D::GenerateContacts(std::vector<ContactManifold2D>& manifolds)
	{	
		if (!BoxHalfSpaceCollision2D(*m_Box, *m_Edge)) return 0;

		// TODO: do not do it twice (first time in line above).
		// Transform to world space.
		glm::vec2 edgeStart = m_Edge->GetAttachedRigidBody()->TransformToWorld(m_Edge->Start);
		glm::vec2 edgeEnd = m_Edge->GetAttachedRigidBody()->TransformToWorld(m_Edge->End);
		glm::vec2 boxCenter = m_Box->GetAttachedRigidBody()->TransformToWorld(m_Box->Center);

		// Check every vertex against edge.
		glm::vec2 vertices[4] = {
			boxCenter + glm::vec2{-m_Box->HalfSize.x, -m_Box->HalfSize.y},
			boxCenter + glm::vec2{ m_Box->HalfSize.x, -m_Box->HalfSize.y},
			boxCenter + glm::vec2{ m_Box->HalfSize.x,  m_Box->HalfSize.y},
			boxCenter + glm::vec2{-m_Box->HalfSize.x,  m_Box->HalfSize.y}
		};
		
		glm::vec2 edgeDir = edgeEnd - edgeStart;
		glm::vec2 normal = glm::vec2{ -edgeDir.y, edgeDir.x };
		normal = glm::normalize(normal);
		F32 offset = -glm::dot(normal, edgeStart);

		ContactManifold2D manifold;
		manifold.Bodies[0] = const_cast<RigidBody2D*>(m_Edge->GetAttachedRigidBody());
		manifold.Bodies[1] = const_cast<RigidBody2D*>(m_Box->GetAttachedRigidBody()); manifold.ContactCount = 0;
		manifold.ContactNormal = normal;
		manifold.ContactCount = 0;
		for (U32 i = 0; i < 4; i++)
		{
			F32 vertexDistance = glm::dot(vertices[i], normal);
			if (vertexDistance <= offset)
			{
				// Create contact data.
				ContactInfo2D contactInfo;
				contactInfo.Point = normal * (vertexDistance - offset) + vertices[i];
				contactInfo.PenetrationDepth = offset - vertexDistance;
				manifold.Contacts[manifold.ContactCount] = contactInfo;
				manifold.ContactCount++;
				if (manifold.ContactCount == 2) break;
			}
		}

		manifolds.push_back(manifold);
		return manifold.ContactCount;
	}

	ContactRegistation ContactManager::s_Registry
		[static_cast<U32>(Collider2D::Type::TypesCount)]
		[static_cast<U32>(Collider2D::Type::TypesCount)];

	bool ContactManager::s_IsInit = false;

	Contact2D* ContactManager::Create(Collider2D* a, Collider2D* b)
	{
		if (!s_IsInit)
		{
			Init();
			s_IsInit = true;
		}

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

	void ContactManager::Destroy(Contact2D* contact)
	{
		std::array<Collider2D*, 2> colliders = contact->GetColliders();
		U32 aTypeInt = colliders[0]->GetTypeInt();
		U32 bTypeInt = colliders[1]->GetTypeInt();
		s_Registry[aTypeInt][bTypeInt].DestroyFn(contact);
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

	void ContactResolver::Resolve(const ContactManifold2D& manifold)
	{
		RigidBody2D* first  = manifold.Bodies[0];
		RigidBody2D* second = manifold.Bodies[1];
		//x temp
		if (!(first->HasFiniteMass() || second->HasFiniteMass())) return;
		//! This requires some rework.
		for (U32 i = 0; i < manifold.ContactCount; i++)
		{
			glm::vec2 relativeVel = first->GetLinearVelocity() - second->GetLinearVelocity();
			F32 jv = glm::dot(relativeVel, manifold.ContactNormal);
			// Collision resolves by itself.
			if (jv > 0) return;

			F32 baumgarte = 0.01f / (1.0f / 60.0f) * manifold.Contacts[i].PenetrationDepth;
			F32 restitution = manifold.GetRestitution();
			//F32 impulse = -(1.0f + restitution) * jv + baumgarte;
			F32 impulse = -(1.0f + restitution) * jv;
			impulse /= first->GetInverseMass() + second->GetInverseMass();

			first->SetLinearVelocity(first->GetLinearVelocity() + impulse * first->GetInverseMass() * manifold.ContactNormal);
			second->SetLinearVelocity(second->GetLinearVelocity() - impulse * second->GetInverseMass() * manifold.ContactNormal);

			//x Very temporal thing.
			F32 totalInverseMass = first->GetInverseMass() + second->GetInverseMass();

			// Find the amount of penetration resolution per unit of inverse mass.
			glm::vec2 movePerInvMass = manifold.ContactNormal * (manifold.Contacts[i].PenetrationDepth / totalInverseMass);
			// Apply the penetration resolution.
			first->SetPosition(first->GetPosition() + movePerInvMass * first->GetInverseMass());
			second->SetPosition(second->GetPosition() - movePerInvMass * second->GetInverseMass());
		}
		

		//if (!contactInfo.Bodies.First->HasFiniteMass() && !contactInfo.Bodies.Second->HasFiniteMass()) return;
		//const glm::vec2& normal = contactInfo.Normal;
		//// TODO: use collider center instead of body center?
		//// Calculate collider centers relative to the contact.
		//glm::vec2 relPos[2];
		//relPos[0] = contactInfo.Point - glm::vec2(contactInfo.Bodies.First->GetPosition());
		//if (contactInfo.Bodies.Second != nullptr)
		//{
		//	relPos[1] = contactInfo.Point - glm::vec2(contactInfo.Bodies.Second->GetPosition());
		//}
		//// If my math is correct (it is), lines below are always zero.
		///* 
		//glm::vec2 relativePos = contactInfo.Point - glm::vec2(contactInfo.Bodies.First->GetPosition());
		//// R x N x R (with 2d magic).
		//glm::vec2 deltaVelWorldVec{
		//	relativePos.x * normal.y * normal.y - relativePos.y * normal.x * normal.y,
		//	relativePos.y * normal.x * normal.x - relativePos.x * normal.y * normal.x
		//};
		//deltaVelWorldVec *= contactInfo.Bodies.First->GetInverseInertia();
		//F32 deltaVel = glm::dot(deltaVelWorldVec, normal);
		//*/

		//F32 deltaVel = contactInfo.Bodies.First->GetInverseMass();
		//if (contactInfo.Bodies.Second != nullptr)
		//{
		//	deltaVel += contactInfo.Bodies.Second->GetInverseMass();
		//}

		//// Get closing velocity as sum of transverse and radial.
		//glm::vec2 closingVelWorld =
		//	Math::Cross2D(contactInfo.Bodies.First->GetAngularVelocity(), relPos[0]);
		//closingVelWorld += contactInfo.Bodies.First->GetLinearVelocity();
		//
		//if (contactInfo.Bodies.Second != nullptr)
		//{
		//	closingVelWorld -=
		//		Math::Cross2D(contactInfo.Bodies.Second->GetAngularVelocity(), relPos[1]);
		//	closingVelWorld -= contactInfo.Bodies.Second->GetLinearVelocity();
		//}
		//
		//// Get closing velocity in contact's coordinate system.
		//// NOTE: in frictionless collisions we only need the x component,
		//// so we can simplify it to be n dot v.
		//glm::vec2 contactVel {
		//	 normal.x * closingVelWorld.x + normal.y * closingVelWorld.y,
		//	-normal.y * closingVelWorld.x + normal.x * closingVelWorld.y,
		//};
		//// TODO: make the whole method depend on time.
		//F32 deltaTime = 1.0f / 60.0f; // TODO: temp.
		//// The change in velocity we need, to resolve contact.
		//F32 desiredDeltaVel = -contactVel.x * (1.0f + contactInfo.GetRestitution()) * deltaTime;
		//// Calculate impuse in contact's cooridinate system.
		//glm::vec2 contactImpulse { desiredDeltaVel / deltaVel, 0.0f };
		//// Convert it to world space (explicit matrix by vector mult).
		//glm::vec2 impulse {
		//	normal.x * contactImpulse.x - normal.y * contactImpulse.y,
		//	normal.y * contactImpulse.x + normal.x * contactImpulse.y,
		//};
		//// Calculate velocity changes according to the impulse.
		//glm::vec2 linearVelChange = impulse * contactInfo.Bodies.First->GetInverseMass();
		//F32 angularVelChange = Math::Cross2D(impulse, relPos[0]) * contactInfo.Bodies.First->GetInverseInertia();
		//contactInfo.Bodies.First->SetLinearVelocity(contactInfo.Bodies.First->GetLinearVelocity() + linearVelChange);
		//contactInfo.Bodies.First->SetAngularVelocity(contactInfo.Bodies.First->GetAngularVelocity() + angularVelChange);

		//if (contactInfo.Bodies.Second != nullptr)
		//{
		//	impulse = -impulse;
		//	glm::vec2 linearVelChange = impulse * contactInfo.Bodies.Second->GetInverseMass();
		//	F32 angularVelChange = Math::Cross2D(impulse, relPos[0]) * contactInfo.Bodies.Second->GetInverseInertia();
		//	contactInfo.Bodies.Second->SetLinearVelocity(contactInfo.Bodies.Second->GetLinearVelocity() + linearVelChange);
		//	contactInfo.Bodies.Second->SetAngularVelocity(contactInfo.Bodies.Second->GetAngularVelocity() + angularVelChange);
		//}


		//if (contactInfo.PenetrationDepth <= 0.0f) return;

		//// Move each object relative to its mass.
		//F32 totalInverseMass = contactInfo.Bodies.First->GetInverseMass();
		//if (contactInfo.Bodies.Second) totalInverseMass += contactInfo.Bodies.Second->GetInverseMass();

		//// If all particles have infinite mass, nothing can be done.
		//if (totalInverseMass <= 0) return;

		//// Find the amount of penetration resolution per unit of inverse mass.
		//glm::vec2 movePerInvMass = contactInfo.Normal * (contactInfo.PenetrationDepth / totalInverseMass);

		//// Apply the penetration resolution.
		//contactInfo.Bodies.First->SetPosition(glm::vec3((glm::vec2(contactInfo.Bodies.First->GetPosition()) + movePerInvMass * contactInfo.Bodies.First->GetInverseMass()), 0.0f));
		//// Particles[1]'s resolution is in opposite direction (if Particles[1] is present).
		//if (contactInfo.Bodies.Second) contactInfo.Bodies.Second->SetPosition(glm::vec3((glm::vec2(contactInfo.Bodies.Second->GetPosition()) - movePerInvMass * contactInfo.Bodies.Second->GetInverseMass()), 0.0f));
	}

}


