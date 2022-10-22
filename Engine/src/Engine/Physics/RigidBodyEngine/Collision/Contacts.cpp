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
		F32 radius = m_First->Radius + m_Second->Radius;
		// SAT.
		SATQuery satA = SATFaceDirections(*m_First, *m_Second, 
			m_First->GetAttachedRigidBody()->GetTransform(), m_Second->GetAttachedRigidBody()->GetTransform());
		if (satA.Distance > radius) return 0;

		SATQuery satB = SATFaceDirections(*m_Second, *m_First,
			m_Second->GetAttachedRigidBody()->GetTransform(), m_First->GetAttachedRigidBody()->GetTransform());
		if (satB.Distance > radius) return 0;
		
		// We need the axis with smallest depth.
		SATQuery sat;
		BoxCollider2D* primary = nullptr;
		BoxCollider2D* secondary = nullptr;
		// Add some bias to satA for coherency.
		static constexpr auto satBias = 0.005f;
		if (satA.Distance > satB.Distance - satBias)
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
		LineSegment2D incidentFace{
			.Start = secondary->GetVertex(incidentFaceI),
			.End = secondary->GetVertex(incidentFaceI < 3 ? incidentFaceI + 1 : 0)
		};
		Line2D sideA{
			.Offset = -glm::dot(primary->GetVertex(sat.FaceIndex), refFaceDir) + radius,
			.Normal = refFaceDir
		};
		Line2D sideB{
			.Offset = glm::dot(primary->GetVertex(sat.FaceIndex < 3 ? sat.FaceIndex + 1 : 0), refFaceDir) + radius,
			.Normal = -refFaceDir
		};

		ClipLineSegmentToLine(incidentFace, incidentFace, sideA);
		ClipLineSegmentToLine(incidentFace, incidentFace, sideB);

		// Keep all points below reference face.
		std::array<glm::vec2, 2> clippedPoints{ incidentFace.Start, incidentFace.End };
		glm::vec2 refFaceNormal = glm::vec2{ -refFaceDir.y, refFaceDir.x };
		F32 refFaceOffset = -glm::dot(refFaceNormal, primary->GetVertex(sat.FaceIndex));

		info.Colliders[0] = primary;
		info.Colliders[1] = secondary;
		manifold.LocalNormal = primary->GetAttachedRigidBody()->GetTransform().InverseTransformDirection(-refFaceNormal);
		manifold.ContactCount = 0;
		manifold.LocalReferencePoint = (primary->GetVertex(sat.FaceIndex) + primary->GetVertex(sat.FaceIndex < 3 ? sat.FaceIndex + 1 : 0)) * 0.5f;
		manifold.LocalReferencePoint = primary->GetAttachedRigidBody()->GetTransform().InverseTransform(manifold.LocalReferencePoint);

		for (U32 i = 0; i < clippedPoints.size(); i++)
		{
			// Compute distance to reference face.
			F32 distance = glm::dot(clippedPoints[i], refFaceNormal) + refFaceOffset;
			if (distance > radius) continue;

			ContactPoint2D contactInfo{};
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
		
		info.Colliders[0] = m_First;
		info.Colliders[1] = m_Second;
		manifold.LocalNormal = normal;
		manifold.ContactCount = 1;
		ContactPoint2D contactInfo{};
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

		info.Colliders[0] = m_Circle;
		info.Colliders[1] = m_Box;
		manifold.ContactCount = 1;
		manifold.LocalNormal = -closestPointWorld  + circleCenter;
		if (manifold.LocalNormal.x != 0.0 || manifold.LocalNormal.y != 0)
			manifold.LocalNormal = glm::normalize(manifold.LocalNormal);
		
		ContactPoint2D contactInfo{};
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

		ContactPoint2D contactInfo{};
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

		info.Colliders[0] = m_Edge;
		info.Colliders[1] = m_Circle;
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

		info.Colliders[0] = m_Edge;
		info.Colliders[1] = m_Box;
		manifold.ContactCount = 0;
		manifold.LocalNormal = normal;
		manifold.ContactCount = 0;
		for (U32 i = 0; i < 4; i++)
		{
			F32 vertexDistance = glm::dot(vertices[i], normal);
			if (vertexDistance <= offset)
			{
				// Create contact data.
				ContactPoint2D contactInfo{};
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
	std::vector<glm::vec2> ContactResolver::s_ContactPoints;
	std::vector<glm::vec2> ContactResolver::s_ContactNormals;
	bool ContactResolver::s_StoreContactPointInfo = false;

	void ContactResolver::PreSolve(const ContactResolverDef& crDef)
	{
		if (s_StoreContactPointInfo)
		{
			s_ContactPoints.clear();
			s_ContactNormals.clear();
		}
		
		s_ContactConstraints.clear();

		s_ContactConstraints.reserve(crDef.ContactListSize);
		
		ContactInfoEntry2D* currentInfo = crDef.ContactList;
		for (ContactInfoEntry2D* currentInfo = crDef.ContactList; currentInfo != nullptr; currentInfo = currentInfo->Next)
		{
			if (currentInfo->Info.Manifold != nullptr)
			{
				// We need not resolve sensors.
				if (currentInfo->Info.HasSensors())
				{
					continue;
				}

				ContactManifold2D& manifold = *currentInfo->Info.Manifold;
				s_ContactConstraints.push_back(ContactConstraint2D{
					.ContactInfo = &currentInfo->Info });
				const RigidBody2D*  first = currentInfo->Info.Colliders[0]->GetAttachedRigidBody();
				const RigidBody2D* second = currentInfo->Info.Colliders[1]->GetAttachedRigidBody();
				const Transform2D& tfA = first->GetTransform();
				const Transform2D& tfB = second->GetTransform();
				glm::vec2 centerOfMassA = tfA.Transform(first->GetCenterOfMass());
				glm::vec2 centerOfMassB = tfB.Transform(second->GetCenterOfMass());
				glm::vec2 normal = tfA.TransformDirection(manifold.LocalNormal);
				glm::vec2 tangent{ -normal.y, normal.x };
				for (U32 i = 0; i < manifold.ContactCount; i++)
				{
					glm::vec2 contactPointWorld = tfB.Transform(manifold.Contacts[i].LocalPoint);
					glm::vec2 distA = contactPointWorld - centerOfMassA;
					glm::vec2 distB = contactPointWorld - centerOfMassB;
					F32 tDistA = Math::Cross2D(distA, tangent);
					F32 tDistB = Math::Cross2D(distB, tangent);
					F32 nDistA = Math::Cross2D(distA, normal);
					F32 nDistB = Math::Cross2D(distB, normal);

					F32 commonMass = first->GetInverseMass() + second->GetInverseMass();

					s_ContactConstraints.back().DistVecA[i] = distA;
					s_ContactConstraints.back().DistVecB[i] = distB;

					s_ContactConstraints.back().NormalMasses[i] = commonMass + 
						 first->GetInverseInertia() * nDistA * nDistA +
						second->GetInverseInertia() * nDistB * nDistB;

					s_ContactConstraints.back().TangentMasses[i] = commonMass +
						 first->GetInverseInertia() * tDistA * tDistA +
						second->GetInverseInertia() * tDistB * tDistB;

					s_ContactConstraints.back().NormalMasses[i] = 1.0f / s_ContactConstraints.back().NormalMasses[i];
					s_ContactConstraints.back().TangentMasses[i] = 1.0f / s_ContactConstraints.back().TangentMasses[i];

					if (crDef.WarmStartEnabled == false)
					{
						s_ContactConstraints.back().ContactInfo->AccumulatedNormalImpulses[i] = 0.0f;
						s_ContactConstraints.back().ContactInfo->AccumulatedTangentImpulses[i] = 0.0f;
					}
				}
			}
		}
	}

	void ContactResolver::WarmStart()
	{
		for (auto& constraint : s_ContactConstraints)
		{
			ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
			RigidBody2D* first =  const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[0]->GetAttachedRigidBody());
			RigidBody2D* second = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[1]->GetAttachedRigidBody());
			const Transform2D& tfA = first->GetTransform();
			const Transform2D& tfB = second->GetTransform();
			glm::vec2 centerOfMassA = tfA.Transform(first->GetCenterOfMass());
			glm::vec2 centerOfMassB = tfB.Transform(second->GetCenterOfMass());

			if (first->GetType() == RigidBodyType2D::Static && second->GetType() == RigidBodyType2D::Static) continue;

			for (U32 i = 0; i < manifold.ContactCount; i++)
			{
				// TODO: store in constraint.
				glm::vec2 contactPointWorld = tfB.Transform(manifold.Contacts[i].LocalPoint);
				glm::vec2 distA = contactPointWorld - centerOfMassA;
				glm::vec2 distB = contactPointWorld - centerOfMassB;
				glm::vec2 normal = first->GetTransform().TransformDirection(manifold.LocalNormal);
				glm::vec2 tangent{ -normal.y, normal.x };

				glm::vec2 impulseVec = normal * constraint.ContactInfo->AccumulatedNormalImpulses[i] +
					tangent * constraint.ContactInfo->AccumulatedTangentImpulses[i];

				first->SetLinearVelocity(first->GetLinearVelocity() + impulseVec * first->GetInverseMass());
				first->SetAngularVelocity(first->GetAngularVelocity() + Math::Cross2D(distA, impulseVec) * first->GetInverseInertia());
				second->SetLinearVelocity(second->GetLinearVelocity() - impulseVec * second->GetInverseMass());
				second->SetAngularVelocity(second->GetAngularVelocity() - Math::Cross2D(distB, impulseVec) * second->GetInverseInertia());
			}

		}
	}

	void ContactResolver::ResolveTangentVelocity(const ContactConstraint2D& constraint)
	{
		RigidBody2D* first = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[0]->GetAttachedRigidBody());
		RigidBody2D* second = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[1]->GetAttachedRigidBody());
		const Transform2D& tfA = first->GetTransform();
		const Transform2D& tfB = second->GetTransform();
		const ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;

		for (U32 i = 0; i < manifold.ContactCount; i++)
		{
			glm::vec2 normal = tfA.TransformDirection(manifold.LocalNormal);
			glm::vec2 tangent{ -normal.y, normal.x };
			glm::vec2 distA = constraint.DistVecA[i];
			glm::vec2 distB = constraint.DistVecB[i];

			glm::vec2 relativeVel =
				  first->GetLinearVelocity() + Math::Cross2D( first->GetAngularVelocity(), distA) -
				(second->GetLinearVelocity() + Math::Cross2D(second->GetAngularVelocity(), distB));
			// TODO: account for tangent speed (for revoluting bodies).
			F32 jv = glm::dot(relativeVel, tangent);

			F32 effectiveMass = constraint.TangentMasses[i];
			F32 deltaImpulse = -jv * effectiveMass;

			F32 maxFriction = constraint.ContactInfo->GetFriction() * constraint.ContactInfo->AccumulatedNormalImpulses[i];
			F32 newImpulse = Math::Clamp(constraint.ContactInfo->AccumulatedTangentImpulses[i] + deltaImpulse, -maxFriction, maxFriction);
			deltaImpulse = newImpulse - constraint.ContactInfo->AccumulatedTangentImpulses[i];
			constraint.ContactInfo->AccumulatedTangentImpulses[i] = newImpulse;

			glm::vec2 impulseVec = deltaImpulse * tangent;

			first->SetLinearVelocity(first->GetLinearVelocity() + impulseVec * first->GetInverseMass());
			first->SetAngularVelocity(first->GetAngularVelocity() + Math::Cross2D(distA, impulseVec) * first->GetInverseInertia());
			second->SetLinearVelocity(second->GetLinearVelocity() - impulseVec * second->GetInverseMass());
			second->SetAngularVelocity(second->GetAngularVelocity() - Math::Cross2D(distB, impulseVec) * second->GetInverseInertia());
		}
	}

	void ContactResolver::ResolveNormalVelocity(const ContactConstraint2D& constraint)
	{
		RigidBody2D* first = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[0]->GetAttachedRigidBody());
		RigidBody2D* second = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[1]->GetAttachedRigidBody());
		const Transform2D& tfA =  first->GetTransform();
		const Transform2D& tfB = second->GetTransform();
		const ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;

		for (U32 i = 0; i < manifold.ContactCount; i++)
		{
			glm::vec2 normal = tfA.TransformDirection(manifold.LocalNormal);
			glm::vec2 distA = constraint.DistVecA[i];
			glm::vec2 distB = constraint.DistVecB[i];

			glm::vec2 relativeVel =
				  first->GetLinearVelocity() + Math::Cross2D( first->GetAngularVelocity(), distA) -
				(second->GetLinearVelocity() + Math::Cross2D(second->GetAngularVelocity(), distB));
			F32 jv = glm::dot(relativeVel, normal);

			F32 effectiveMass = constraint.NormalMasses[i];
			F32 restitution = constraint.ContactInfo->GetRestitution();

			F32 deltaImpulse = -(1.0f + restitution) * jv * effectiveMass;
			F32 newImpulse = Math::Max(constraint.ContactInfo->AccumulatedNormalImpulses[i] + deltaImpulse, 0.0f);
			deltaImpulse = newImpulse - constraint.ContactInfo->AccumulatedNormalImpulses[i];
			constraint.ContactInfo->AccumulatedNormalImpulses[i] = newImpulse;
			
			glm::vec2 impulseVec = deltaImpulse * normal;

			first->SetLinearVelocity(first->GetLinearVelocity() + impulseVec * first->GetInverseMass());
			first->SetAngularVelocity(first->GetAngularVelocity() + Math::Cross2D(distA, impulseVec) * first->GetInverseInertia());
			second->SetLinearVelocity(second->GetLinearVelocity() - impulseVec * second->GetInverseMass());
			second->SetAngularVelocity(second->GetAngularVelocity() - Math::Cross2D(distB, impulseVec) * second->GetInverseInertia());
		}
	}

	void ContactResolver::ResolveVelocity()
	{
		for (auto& constraint : s_ContactConstraints)
		{
			ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
			RigidBody2D* first = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[0]->GetAttachedRigidBody());
			RigidBody2D* second = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[1]->GetAttachedRigidBody());
			
			if (first->GetType() == RigidBodyType2D::Static && second->GetType() == RigidBodyType2D::Static) continue;

			ResolveTangentVelocity(constraint);
			ResolveNormalVelocity(constraint);
		}
	}

	bool ContactResolver::ResolvePosition()
	{
		static constexpr auto baumgarte = 0.2f;
		F32 maxDepth = -std::numeric_limits<F32>::max();
		for (auto& constraint : s_ContactConstraints)
		{
			ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
			RigidBody2D* first = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[0]->GetAttachedRigidBody());
			RigidBody2D* second = const_cast<RigidBody2D*>(constraint.ContactInfo->Colliders[1]->GetAttachedRigidBody());
			const Transform2D& tfA = first->GetTransform();
			const Transform2D& tfB = second->GetTransform();
			glm::vec2 centerOfMassA = tfA.Transform(first->GetCenterOfMass());
			glm::vec2 centerOfMassB = tfB.Transform(second->GetCenterOfMass());

			for (U32 i = 0; i < manifold.ContactCount; i++)
			{
				glm::vec2 normal = tfA.TransformDirection(manifold.LocalNormal);
				glm::vec2 refPoint = tfA.Transform(manifold.LocalReferencePoint);

				// TODO: store in constraint.
				glm::vec2 contactPointWorld = tfB.Transform(manifold.Contacts[i].LocalPoint);
				if (s_StoreContactPointInfo)
				{
					s_ContactPoints.push_back(contactPointWorld);
					s_ContactNormals.push_back(normal);
				}
				

				// Can't use ones stored in constraint, because position changes.
				glm::vec2 distA = contactPointWorld - centerOfMassA;
				glm::vec2 distB = contactPointWorld - centerOfMassB;
				F32 nDistA = Math::Cross2D(distA, normal);
				F32 nDistB = Math::Cross2D(distB, normal);

				F32 effectiveDepth = manifold.Contacts[i].PenetrationDepth;
				effectiveDepth = glm::dot((contactPointWorld - refPoint), normal);
				if (effectiveDepth < 0.0f) continue;
				maxDepth = Math::Max(maxDepth, effectiveDepth);

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
		return maxDepth < 0.005f * 3.0f;
	}

	F32 ContactResolver::GetDeltaImpulse(const ContactInfo2D& info, U32 contactIndex)
	{
		RigidBody2D* first = const_cast<RigidBody2D*>(info.Colliders[0]->GetAttachedRigidBody());
		RigidBody2D* second = const_cast<RigidBody2D*>(info.Colliders[1]->GetAttachedRigidBody());
		const Transform2D& tfA = first->GetTransform();
		const Transform2D& tfB = second->GetTransform();
		glm::vec2 centerOfMassA = tfA.Transform(first->GetCenterOfMass());
		glm::vec2 centerOfMassB = tfB.Transform(second->GetCenterOfMass());

		if (first->GetType() == RigidBodyType2D::Static && second->GetType() == RigidBodyType2D::Static) return 0.0f;
		ContactManifold2D& manifold = *info.Manifold;
		glm::vec2 normal = first->GetTransform().TransformDirection(manifold.LocalNormal);

		// TODO: store in constraint.
		glm::vec2 contactPointWorld = tfB.Transform(manifold.Contacts[contactIndex].LocalPoint);
		glm::vec2 distA = contactPointWorld - centerOfMassA;
		glm::vec2 distB = contactPointWorld - centerOfMassB;
		F32 nDistA = Math::Cross2D(distA, normal);
		F32 nDistB = Math::Cross2D(distB, normal);

		glm::vec2 relativeVel =
			  first->GetLinearVelocity() + Math::Cross2D( first->GetAngularVelocity(), distA) -
			(second->GetLinearVelocity() + Math::Cross2D(second->GetAngularVelocity(), distB));
		F32 jv = glm::dot(relativeVel, normal);

		F32 effectiveMass = first->GetInverseMass() + second->GetInverseMass() +
			 first->GetInverseInertia() * nDistA * nDistA +
			second->GetInverseInertia() * nDistB * nDistB;
		F32 restitution = info.GetRestitution();
		F32 impulse = -(1.0f + restitution) * jv / effectiveMass;
		
		return impulse;
	}
	
	void ContactResolver::PostSolve()
	{
		// TODO: put objects to sleep.
	}

	DefaultContactListener* DefaultContactListener::s_Instance = nullptr;

	void DefaultContactListener::Init()
	{
		ENGINE_CORE_ASSERT(s_Instance == nullptr, "Default contact listener is already created.");
		s_Instance = New<DefaultContactListener>();
	}

	void DefaultContactListener::Shutdown()
	{
		Delete<DefaultContactListener>(s_Instance);
	}

	DefaultContactListener* DefaultContactListener::Get()
	{
		if (s_Instance == nullptr)
			Init();
		return s_Instance;
	}

}


