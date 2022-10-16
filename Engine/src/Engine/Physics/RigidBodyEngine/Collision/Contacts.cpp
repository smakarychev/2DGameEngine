#include "enginepch.h"

#include "Contacts.h"
#include "NarrowPhase.h"

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

	U32 BoxBoxContact2D::GenerateContacts(ContactInfo2D& info)
	{
		ContactManifold2D& manifold = *info.Manifold;
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
		static constexpr auto satBias = 0.0003f;
		if (satB.Distance > satA.Distance - satBias)
		{
			sat = satB;
			primary = m_Second;
			secondary = m_First;
		}
		else
		{
			sat = satA;
			primary = m_First;
			secondary = m_Second;
		}
		glm::vec2 refFaceDir = primary->GetFaceDirection(sat.FaceIndex);
		I32 incidentFaceI = FindIncidentFaceIndex(*secondary,
			refFaceDir,
			secondary->GetAttachedRigidBody()->GetTransform());
		glm::vec2 incidentFaceDir = secondary->GetFaceDirection(incidentFaceI);


		// Clip incident face by side planes of reference face.
		// Note that ref face in a normal / anti-normal of side planes.
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

		info.Bodies[0] = const_cast<RigidBody2D*>(primary->GetAttachedRigidBody());
		info.Bodies[1] = const_cast<RigidBody2D*>(secondary->GetAttachedRigidBody());
		manifold.LocalNormal = primary->GetAttachedRigidBody()->GetTransform().InverseTransformDirection(-refFaceNormal);
		manifold.ContactCount = 0;
		manifold.LocalReferencePoint = (primary->GetVertex(sat.FaceIndex) + primary->GetVertex(sat.FaceIndex < 3 ? sat.FaceIndex + 1 : 0)) * 0.5f;
		manifold.LocalReferencePoint = primary->GetAttachedRigidBody()->GetTransform().InverseTransform(manifold.LocalReferencePoint);

		for (U32 i = 0; i < clippedPoints.size(); i++)
		{
			// Compute distance to reference face.
			F32 distance = glm::dot(clippedPoints[i], refFaceNormal) + refFaceOffset;
			if (distance > 0.0f) continue;
			//TODO: Move contact point onto the ref face (this helps coherence)?

			ContactPoint2D contactInfo;
			contactInfo.LocalPoint = secondary->GetAttachedRigidBody()->GetTransform().InverseTransform(clippedPoints[i]);
			contactInfo.PenetrationDepth = -distance;
			manifold.Contacts[manifold.ContactCount] = contactInfo;
			manifold.ContactCount++;
		}

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

	U32 CircleCircleContact2D::GenerateContacts(ContactInfo2D& info)
	{
		ENGINE_CORE_ERROR("Circle - Circle contact does not work correctly!");
		ContactManifold2D& manifold = *info.Manifold;
		manifold.ContactCount = 0;
		// Transform positions to world space.
		glm::vec2 firstCenter = m_First->GetAttachedRigidBody()->TransformToWorld(m_First->Center);
		glm::vec2 secondCenter = m_Second->GetAttachedRigidBody()->TransformToWorld(m_Second->Center);

		// Check if there is a collision.
		glm::vec2 distVec = firstCenter - secondCenter;
		F32 minDist = m_First->Radius + m_Second->Radius;
		F32 minDistSquared = minDist * minDist;
		F32 distSquared = glm::length2(distVec);
		if (distSquared > minDistSquared) return 0;
		
		// Else we have a collision.
		F32 dist = Math::Sqrt(distSquared);
		glm::vec2 normal = distVec / dist;
		
		info.Bodies[0] = const_cast<RigidBody2D*>(m_First->GetAttachedRigidBody());
		info.Bodies[1] = const_cast<RigidBody2D*>(m_Second->GetAttachedRigidBody());
		manifold.LocalNormal = normal;
		manifold.ContactCount = 1;
		ContactPoint2D contactInfo;
		contactInfo.LocalPoint = m_Second->GetAttachedRigidBody()->GetTransform().InverseTransform(firstCenter + distVec * 0.5f);
		contactInfo.PenetrationDepth = minDist - dist;
		manifold.Contacts[0] = contactInfo;

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

	U32 BoxCircleContact2D::GenerateContacts(ContactInfo2D& info)
	{
		ContactManifold2D& manifold = *info.Manifold;
		manifold.ContactCount = 0;
		ENGINE_CORE_ERROR("Box - Circle contact does not work correctly!");
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
		glm::vec2 closestPointWorld = m_Box->GetAttachedRigidBody()->TransformToWorld(closestPoint);

		info.Bodies[0] = const_cast<RigidBody2D*>(m_Circle->GetAttachedRigidBody());
		info.Bodies[1] = const_cast<RigidBody2D*>(m_Box->GetAttachedRigidBody());
		manifold.ContactCount = 1;
		manifold.LocalNormal = -closestPointWorld  + circleCenter;
		if (manifold.LocalNormal.x != 0.0 || manifold.LocalNormal.y != 0)
			manifold.LocalNormal = glm::normalize(manifold.LocalNormal);
		
		ContactPoint2D contactInfo;
		contactInfo.LocalPoint= closestPoint;
		contactInfo.PenetrationDepth = m_Circle->Radius - Math::Sqrt(distanceSquared);
		manifold.Contacts[0] = contactInfo;

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
	
	U32 EdgeCircleContact2D::GenerateContacts(ContactInfo2D& info)
	{
		ContactManifold2D& manifold = *info.Manifold;
		manifold.ContactCount = 0;
		ENGINE_CORE_ERROR("Edge - Circle contact does not work correctly!");
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

		ContactPoint2D contactInfo;
		contactInfo.LocalPoint= m_Circle->GetAttachedRigidBody()->GetTransform().InverseTransform(circleCenter - normal * distanceToPlane);
		
		F32 depth = -distanceToPlane;
		// If circle is on other side of the edge.
		if (distanceToPlane < 0)
		{
			normal = -normal;
			depth = -depth;
		}
		depth += m_Circle->Radius;

		contactInfo.PenetrationDepth = depth;

		info.Bodies[0] = const_cast<RigidBody2D*>(m_Edge->GetAttachedRigidBody());
		info.Bodies[1] = const_cast<RigidBody2D*>(m_Circle->GetAttachedRigidBody());
		manifold.LocalNormal = normal;
		manifold.ContactCount = 1;
		manifold.Contacts[0] = contactInfo;

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
	
	U32 EdgeBoxContact2D::GenerateContacts(ContactInfo2D& info)
	{	
		ContactManifold2D& manifold = *info.Manifold;
		ENGINE_CORE_ERROR("Edge - box contact does not work correctly!");
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

		info.Bodies[0] = const_cast<RigidBody2D*>(m_Edge->GetAttachedRigidBody());
		info.Bodies[1] = const_cast<RigidBody2D*>(m_Box->GetAttachedRigidBody()); manifold.ContactCount = 0;
		manifold.LocalNormal = normal;
		manifold.ContactCount = 0;
		for (U32 i = 0; i < 4; i++)
		{
			F32 vertexDistance = glm::dot(vertices[i], normal);
			if (vertexDistance <= offset)
			{
				// Create contact data.
				ContactPoint2D contactInfo;
				contactInfo.LocalPoint= m_Box->GetAttachedRigidBody()->GetTransform().InverseTransform(normal * (vertexDistance - offset) + vertices[i]);
				contactInfo.PenetrationDepth = offset - vertexDistance;
				manifold.Contacts[manifold.ContactCount] = contactInfo;
				manifold.ContactCount++;
				if (manifold.ContactCount == 2) break;
			}
		}

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

	std::vector<ContactConstraint2D> ContactResolver::s_ContactConstraints;
	std::vector<glm::vec2> ContactResolver::s_p;
	std::vector<glm::vec2> ContactResolver::s_n;

	void ContactResolver::PreSolve(ContactInfoNode2D* contactList, U32 constListSize)
	{
		s_p.clear(); //x1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		s_n.clear(); //x1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		s_ContactConstraints.clear();

		s_ContactConstraints.reserve(constListSize);
		
		ContactInfoNode2D* currentInfo = contactList;
		while (currentInfo != nullptr)
		{
			if (currentInfo->Info.Manifold != nullptr)
			{
				ContactManifold2D& manifold = *currentInfo->Info.Manifold;
				s_ContactConstraints.push_back(ContactConstraint2D{
					.ContactInfo = &currentInfo->Info });
			}
			currentInfo = currentInfo->Next;
		}
	}

	void ContactResolver::ResolveVelocity()
	{
		for (auto& constraint : s_ContactConstraints)
		{
			ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
			RigidBody2D* first = constraint.ContactInfo->Bodies[0];
			RigidBody2D* second = constraint.ContactInfo->Bodies[1];
			
			if (first->GetType() == RigidBodyType2D::Static && second->GetType() == RigidBodyType2D::Static) continue;

			//! This requires some rework.
			for (U32 i = 0; i < manifold.ContactCount; i++)
			{
				glm::vec2 normal = first->GetTransform().TransformDirection(manifold.LocalNormal);

				// TODO: store in constraint.
				glm::vec2 contactPointWorld = second->GetTransform().Transform(manifold.Contacts[i].LocalPoint);
				glm::vec2 distA = contactPointWorld - first->GetTransform().Translation;
				glm::vec2 distB = contactPointWorld - second->GetTransform().Translation;

				F32 deltaImpulse = GetDeltaImpulse(*constraint.ContactInfo, i);
				F32 oldImpulse = constraint.ContactInfo->AccumulatedImpulse;
				constraint.ContactInfo->AccumulatedImpulse += deltaImpulse;
				constraint.ContactInfo->AccumulatedImpulse = Math::Max(0.0f, constraint.ContactInfo->AccumulatedImpulse);
				deltaImpulse = constraint.ContactInfo->AccumulatedImpulse - oldImpulse;
				constraint.ContactInfo->AccumulatedImpulse = deltaImpulse;

				first->SetLinearVelocity(first->GetLinearVelocity() + deltaImpulse * first->GetInverseMass() * normal);
				first->SetAngularVelocity(first->GetAngularVelocity() + first->GetInverseInertia() * Math::Cross2D(distA, deltaImpulse * normal));
				second->SetLinearVelocity(second->GetLinearVelocity() - deltaImpulse * second->GetInverseMass() * normal);
				second->SetAngularVelocity(second->GetAngularVelocity() - second->GetInverseInertia() * Math::Cross2D(distB, deltaImpulse * normal));
			}
		}
	}

	bool ContactResolver::ResolvePosition()
	{
		static constexpr auto baumgarte = 0.2f;
		F32 maxDepth = -std::numeric_limits<F32>::max();
		for (auto& constraint : s_ContactConstraints)
		{
			ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
			RigidBody2D* first =  constraint.ContactInfo->Bodies[0];
			RigidBody2D* second = constraint.ContactInfo->Bodies[1];
			if (first->GetType() == RigidBodyType2D::Static && second->GetType() == RigidBodyType2D::Static) continue;
			
			//! This requires some rework.
			for (U32 i = 0; i < manifold.ContactCount; i++)
			{
				glm::vec2 normal = first->GetTransform().TransformDirection(manifold.LocalNormal);
				glm::vec2 refPoint = first->GetTransform().Transform(manifold.LocalReferencePoint);

				// TODO: store in constraint.
				glm::vec2 contactPointWorld = second->GetTransform().Transform(manifold.Contacts[i].LocalPoint);
				s_p.push_back(contactPointWorld); //x1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				s_n.push_back(normal); //x1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

				glm::vec2 distA = contactPointWorld - first->GetTransform().Translation;
				glm::vec2 distB = contactPointWorld - second->GetTransform().Translation;
				F32 nDistA = Math::Cross2D(distA, normal);
				F32 nDistB = Math::Cross2D(distB, normal);

				glm::vec2 centersDistVec = first->GetPosition() - second->GetPosition();
				F32 centersDistNormal = glm::dot(centersDistVec, normal);

				F32 effectiveDepth = manifold.Contacts[i].PenetrationDepth;
				effectiveDepth = glm::dot((contactPointWorld - refPoint), normal);
				maxDepth = Math::Max(maxDepth, effectiveDepth);
				if (effectiveDepth < 0.0f) continue;

				F32 correction = baumgarte * (effectiveDepth - 0.005f);
				correction = Math::Clamp(correction, 0.0f, baumgarte);
				F32 effectiveMass = first->GetInverseMass() + second->GetInverseMass() +
					 first->GetInverseInertia() * nDistA * nDistA +
					second->GetInverseInertia() * nDistB * nDistB;

				F32 impulse = effectiveMass > 0.0f ? correction / effectiveMass : 0.0f;
				glm::vec2 impulseVec = impulse * normal;

				first->SetPosition(first->GetPosition() + impulseVec * first->GetInverseMass());
				first->AddRotation(first->GetInverseInertia() * Math::Cross2D(distA, impulseVec));
				first->SetRotation(glm::normalize(first->GetRotation()));
				second->SetPosition(second->GetPosition() - impulseVec * second->GetInverseMass());
				second->AddRotation(-second->GetInverseInertia() * Math::Cross2D(distB, impulseVec));
				second->SetRotation(glm::normalize(second->GetRotation()));
			}
		}
		return maxDepth < 0.005f * 1.5f;
	}

	void ContactResolver::WarmStart()
	{
		for (auto& constraint : s_ContactConstraints)
		{
			ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
			RigidBody2D* first =  constraint.ContactInfo->Bodies[0];
			RigidBody2D* second = constraint.ContactInfo->Bodies[1];

			glm::vec2 normal = first->GetTransform().TransformDirection(manifold.LocalNormal);

			first->SetLinearVelocity(first->GetLinearVelocity() + constraint.ContactInfo->AccumulatedImpulse * first->GetInverseMass() * normal);
			second->SetLinearVelocity(second->GetLinearVelocity() - constraint.ContactInfo->AccumulatedImpulse * second->GetInverseMass() * normal);
		}
	}

	F32 ContactResolver::GetDeltaImpulse(const ContactInfo2D& info, U32 contactIndex)
	{
		RigidBody2D* first = info.Bodies[0];
		RigidBody2D* second = info.Bodies[1];
		ContactManifold2D& manifold = *info.Manifold;
		glm::vec2 normal = first->GetTransform().TransformDirection(manifold.LocalNormal);

		// TODO: store in constraint.
		glm::vec2 contactPointWorld = second->GetTransform().Transform(manifold.Contacts[contactIndex].LocalPoint);
		glm::vec2 distA = contactPointWorld - first->GetTransform().Translation;
		glm::vec2 distB = contactPointWorld - second->GetTransform().Translation;
		F32 nDistA = Math::Cross2D(distA, normal);
		F32 nDistB = Math::Cross2D(distB, normal);

		glm::vec2 relativeVel =
			 first->GetLinearVelocity()  + Math::Cross2D(first->GetAngularVelocity(), distA) -
			(second->GetLinearVelocity() + Math::Cross2D(second->GetAngularVelocity(), distB));
		F32 jv = glm::dot(relativeVel, normal);

		F32 restitution = info.GetRestitution();
		F32 impulse = -(1.0f + restitution) * jv;
		F32 effectiveMass = first->GetInverseMass() + second->GetInverseMass() +
			first->GetInverseInertia() * nDistA * nDistA +
			second->GetInverseInertia() * nDistB * nDistB;
		impulse /= effectiveMass;
		
		return impulse;
	}

}


