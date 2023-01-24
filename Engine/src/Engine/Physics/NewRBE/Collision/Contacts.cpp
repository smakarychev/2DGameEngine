#include "enginepch.h"

#include "Contacts.h"
#include "NarrowPhase.h"

#include "Engine/Math/LinearAlgebra.h"

namespace Engine::WIP::Physics
{
	PolygonPolygonContact2D::PolygonPolygonContact2D(PolygonCollider2D* first, PolygonCollider2D* second)
		: m_First(first), m_Second(second)
	{}

	U32 PolygonPolygonContact2D::GenerateContacts(ContactInfo2D& info)
	{
		ContactManifold2D& manifold = *info.Manifold;
		F32 radius = m_First->GetRadius() + m_Second->GetRadius();
		// SAT.
		SATQuery satA = SATFaceDirections(*m_First, *m_Second, 
			m_First->GetAttachedTransform(), m_Second->GetAttachedTransform());
		if (satA.Distance > radius) return 0;

		SATQuery satB = SATFaceDirections(*m_Second, *m_First,
			m_Second->GetAttachedTransform(), m_First->GetAttachedTransform());
		if (satB.Distance > radius) return 0;

		auto getVertexTf = [](PolygonCollider2D* col, U32 index)
		{
			return col->GetAttachedTransform().Transform(col->GetVertices()[index]);	
		};
		auto getNormalTf = [](PolygonCollider2D* col, U32 index)
		{
			return col->GetAttachedTransform().TransformDirection(col->GetNormals()[index]);
		};
		
		// We need the axis with the smallest depth.
		SATQuery sat;
		PolygonCollider2D* primary;
		PolygonCollider2D* secondary;
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
		glm::vec2 refFaceDir = Math::Cross2D(getNormalTf(primary, sat.FaceIndex), 1.0f);
		I32 incidentFaceI = FindIncidentFaceIndex(*secondary,
			refFaceDir,
			secondary->GetAttachedTransform());

		// Clip incident face by side planes of reference face.
		// Note that ref face in a normal / anti-normal of side planes.
		LineSegment2D incidentFace{
			.Start = getVertexTf(secondary, incidentFaceI),
			.End = getVertexTf(secondary, incidentFaceI < secondary->GetVertices().size() - 1 ? incidentFaceI + 1 : 0)
		};
		Line2D sideA{
			.Offset = -glm::dot(getVertexTf(primary, sat.FaceIndex), refFaceDir) + radius,
			.Normal = refFaceDir
		};
		Line2D sideB{
			.Offset = glm::dot(getVertexTf(primary, sat.FaceIndex < primary->GetVertices().size() - 1 ? sat.FaceIndex + 1 : 0), refFaceDir) + radius,
			.Normal = -refFaceDir
		};

		ClipLineSegmentToLine(incidentFace, incidentFace, sideA);
		ClipLineSegmentToLine(incidentFace, incidentFace, sideB);

		// Keep all points below reference face.
		std::array<glm::vec2, 2> clippedPoints{ incidentFace.Start, incidentFace.End };
		glm::vec2 refFaceNormal = glm::vec2{ -refFaceDir.y, refFaceDir.x };
		F32 refFaceOffset = -glm::dot(refFaceNormal, getVertexTf(primary, sat.FaceIndex));

		info.Colliders[0] = primary;
		info.Colliders[1] = secondary;
		manifold.LocalNormal = primary->GetAttachedTransform().InverseTransformDirection(-refFaceNormal);
		manifold.ContactCount = 0;
		manifold.LocalReferencePoint = (getVertexTf(primary, sat.FaceIndex) + getVertexTf(primary, sat.FaceIndex < primary->GetVertices().size() - 1 ? sat.FaceIndex + 1 : 0)) * 0.5f;
		manifold.LocalReferencePoint = primary->GetAttachedTransform().InverseTransform(manifold.LocalReferencePoint);

		for (U32 i = 0; i < clippedPoints.size(); i++)
		{
			// Compute distance to reference face.
			F32 distance = glm::dot(clippedPoints[i], refFaceNormal) + refFaceOffset;
			if (distance > radius) continue;

			ContactPoint2D contactInfo{};
			contactInfo.LocalPoint = secondary->GetAttachedTransform().InverseTransform(clippedPoints[i]);
			contactInfo.PenetrationDepth = -distance;
			manifold.Contacts[manifold.ContactCount] = contactInfo;
			manifold.ContactCount++;
		}

		return manifold.ContactCount;
	}

	Contact2D* PolygonPolygonContact2D::Create(NarrowContactAllocator& alloc, Collider2D* a, Collider2D* b)
	{
		return NewAlloc<PolygonPolygonContact2D>(
			alloc,
			static_cast<PolygonCollider2D*>(a),
			static_cast<PolygonCollider2D*>(b)
		);
	}

	CircleCircleContact2D::CircleCircleContact2D(CircleCollider2D* first, CircleCollider2D* second)
		: m_First(first), m_Second(second)
	{
	}

	Contact2D* CircleCircleContact2D::Create(NarrowContactAllocator& alloc, Collider2D* a, Collider2D* b)
	{
		return NewAlloc<CircleCircleContact2D>(
			alloc,
			static_cast<CircleCollider2D*>(a),
			static_cast<CircleCollider2D*>(b)
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
		contactInfo.LocalPoint = m_Second->GetAttachedTransform().InverseTransform(firstCenter + distVec * 0.5f);
		contactInfo.PenetrationDepth = minDist - dist;
		manifold.Contacts[0] = contactInfo;

		return manifold.ContactCount;
	}

	EdgeCircleContact2D::EdgeCircleContact2D(EdgeCollider2D* edge, CircleCollider2D* circle)
		: m_Edge(edge), m_Circle(circle)
	{
	}
	
	Contact2D* EdgeCircleContact2D::Create(NarrowContactAllocator& alloc, Collider2D* a, Collider2D* b)
	{
		return NewAlloc<EdgeCircleContact2D>(
			alloc,
			static_cast<EdgeCollider2D*>(a),
			static_cast<CircleCollider2D*>(b)
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
		contactInfo.LocalPoint= m_Circle->GetAttachedTransform().InverseTransform(circleCenter - normal * distanceToPlane);
		
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
	
	ContactRegistration ContactManager::s_Registry
		[static_cast<U32>(Collider2D::Type::TypesCount)]
		[static_cast<U32>(Collider2D::Type::TypesCount)];

	bool ContactManager::s_IsInit = false;

	Contact2D* ContactManager::Create(NarrowContactAllocator& alloc, Collider2D* a, Collider2D* b)
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
			return s_Registry[aTypeInt][bTypeInt].CreateFn(alloc, a, b);
		}
		else
		{
			return s_Registry[aTypeInt][bTypeInt].CreateFn(alloc, b, a);
		}
	}

	// Create matrix of dipatch functions.
	void ContactManager::Init()
	{
		AddRegistration(PolygonPolygonContact2D::Create, Collider2D::Type::Polygon, Collider2D::Type::Polygon);
		AddRegistration(CircleCircleContact2D::Create, Collider2D::Type::Circle, Collider2D::Type::Circle);
		AddRegistration(EdgeCircleContact2D::Create, Collider2D::Type::Edge, Collider2D::Type::Circle);
	}

	void ContactManager::AddRegistration(
		ContactRegistration::OnCreateFn createFn,
		Collider2D::Type typeA,
		Collider2D::Type typeB
	)
	{
		U32 typeAI = static_cast<U32>(typeA);
		U32 typeBI = static_cast<U32>(typeB);
		s_Registry[typeAI][typeBI].CreateFn = createFn;
		s_Registry[typeAI][typeBI].IsPrimary = true;

		if (typeAI != typeBI)
		{
			s_Registry[typeBI][typeAI].CreateFn = createFn;
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

		for (auto& contactInfoEntry : *crDef.ContactList)
		{
			ContactInfo2D* contactInfo = contactInfoEntry.GetContactInfo();
			if (contactInfo->Manifold != nullptr)
			{
				// We need not resolve sensors.
				if (contactInfo->HasSensors()) continue;
	
				ContactManifold2D& manifold = *contactInfo->Manifold;
				s_ContactConstraints.push_back(ContactConstraint2D{
					.ContactInfo = contactInfo });
				Collider2D* colA = contactInfo->Colliders[0];
				Collider2D* colB = contactInfo->Colliders[1];
				RigidBody2D* rbA = colA->GetAttachedRigidBody();
				RigidBody2D* rbB = colB->GetAttachedRigidBody();

				bool isAImmovable = rbA->GetType() == RigidBodyType2D::Static;
				bool isBImmovable = rbB->GetType() == RigidBodyType2D::Static;
				if (isAImmovable && isBImmovable) continue;
				
				const Transform2D& tfA = colA->GetAttachedTransform();
				const Transform2D& tfB = colB->GetAttachedTransform();
				// CoM is in world coordinates.
				glm::vec2 centerOfMassA = tfA.Transform(rbA->GetCenterOfMass());
				glm::vec2 centerOfMassB = tfB.Transform(rbB->GetCenterOfMass());
				glm::vec2 normal = tfA.TransformDirection(manifold.LocalNormal);
				glm::vec2 tangent{ -normal.y, normal.x };
				
				F32 massInvA = rbA->GetInverseMass();
				F32 massInvB = rbB->GetInverseMass();
				F32 inertInvA = rbA->GetInverseInertia();
				F32 inertInvB = rbB->GetInverseInertia();

				U32 indexOfMaxDepth = manifold.Contacts[0].PenetrationDepth >
				 	manifold.Contacts[1].PenetrationDepth ? 1 : 0;
				glm::vec2 mostDistantPointWorld = tfB.Transform(manifold.Contacts[indexOfMaxDepth].LocalPoint);
			 	glm::vec2 distAMax = mostDistantPointWorld - centerOfMassA;
                glm::vec2 distBMax = mostDistantPointWorld - centerOfMassB;
				glm::vec2 relativeVel{0.0f};
				relativeVel += rbA->GetLinearVelocity() + Math::Cross2D(rbA->GetAngularVelocity(), distAMax);
				relativeVel -= rbB->GetLinearVelocity() + Math::Cross2D(rbB->GetAngularVelocity(), distBMax);
				F32 jv = glm::dot(normal, relativeVel);
				if (jv < -1.0f) s_ContactConstraints.back().VelocityBias = jv * contactInfo->GetRestitution();
				
				for (U32 i = 0; i < manifold.ContactCount; i++)
				{
					glm::vec2 contactPointWorld = tfB.Transform(manifold.Contacts[i].LocalPoint);
					glm::vec2 distA = contactPointWorld - centerOfMassA;
					glm::vec2 distB = contactPointWorld - centerOfMassB;
					F32 tDistA = Math::Cross2D(distA, tangent);
					F32 tDistB = Math::Cross2D(distB, tangent);
					F32 nDistA = Math::Cross2D(distA, normal);
					F32 nDistB = Math::Cross2D(distB, normal);

					F32 commonMass = massInvA + massInvB;

					s_ContactConstraints.back().DistVecA[i] = distA;
					s_ContactConstraints.back().DistVecB[i] = distB;

					s_ContactConstraints.back().NormalMasses[i] = commonMass + 
						inertInvA * nDistA * nDistA +
						inertInvB * nDistB * nDistB;

					s_ContactConstraints.back().TangentMasses[i] = commonMass +
						inertInvA * tDistA * tDistA +
						inertInvB * tDistB * tDistB;

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
			Collider2D* colA = constraint.ContactInfo->Colliders[0];
			Collider2D* colB = constraint.ContactInfo->Colliders[1];
			RigidBody2D* rbA = colA->GetAttachedRigidBody();
			RigidBody2D* rbB = colB->GetAttachedRigidBody();
			
			const Transform2D& tfA = colA->GetAttachedTransform();
			const Transform2D& tfB = colB->GetAttachedTransform();
			
			glm::vec2 centerOfMassA = tfA.Transform(rbA->GetCenterOfMass());
			glm::vec2 centerOfMassB = tfB.Transform(rbB->GetCenterOfMass());

			bool isAImmovable = rbA->GetType() == RigidBodyType2D::Static;
			bool isBImmovable = rbB->GetType() == RigidBodyType2D::Static;
			if (isAImmovable && isBImmovable) continue;

			for (U32 i = 0; i < manifold.ContactCount; i++)
			{
				// TODO: store in constraint.
				glm::vec2 contactPointWorld = tfB.Transform(manifold.Contacts[i].LocalPoint);
				glm::vec2 distA = contactPointWorld - centerOfMassA;
				glm::vec2 distB = contactPointWorld - centerOfMassB;
				glm::vec2 normal = tfA.TransformDirection(manifold.LocalNormal);
				glm::vec2 tangent{ -normal.y, normal.x };

				glm::vec2 impulseVec = normal * constraint.ContactInfo->AccumulatedNormalImpulses[i] +
					tangent * constraint.ContactInfo->AccumulatedTangentImpulses[i];

				rbA->SetLinearVelocity(rbA->GetLinearVelocity() + impulseVec * rbA->GetInverseMass());
				rbA->SetAngularVelocity(rbA->GetAngularVelocity() + Math::Cross2D(distA, impulseVec) * rbA->GetInverseInertia());
				rbB->SetLinearVelocity(rbB->GetLinearVelocity() - impulseVec * rbB->GetInverseMass());
				rbB->SetAngularVelocity(rbB->GetAngularVelocity() - Math::Cross2D(distB, impulseVec) * rbB->GetInverseInertia());
			}
		}
	}

	void ContactResolver::ResolveTangentVelocity(const ContactConstraint2D& constraint)
	{
		const ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
		Collider2D* colA = constraint.ContactInfo->Colliders[0];
		Collider2D* colB = constraint.ContactInfo->Colliders[1];
		RigidBody2D* rbA = colA->GetAttachedRigidBody();
		RigidBody2D* rbB = colB->GetAttachedRigidBody();
		
		const Transform2D& tfA = colA->GetAttachedTransform();

		for (U32 i = 0; i < manifold.ContactCount; i++)
		{
			glm::vec2 normal = tfA.TransformDirection(manifold.LocalNormal);
			glm::vec2 tangent{ -normal.y, normal.x };
			glm::vec2 distA = constraint.DistVecA[i];
			glm::vec2 distB = constraint.DistVecB[i];

			glm::vec2 relativeVel{0.0f};
			relativeVel += rbA->GetLinearVelocity() + Math::Cross2D(rbA->GetAngularVelocity(), distA);
			relativeVel -= rbB->GetLinearVelocity() + Math::Cross2D(rbB->GetAngularVelocity(), distB);
			// TODO: account for tangent speed (for revolting bodies).
			F32 jv = glm::dot(relativeVel, tangent);

			F32 effectiveMass = constraint.TangentMasses[i];
			F32 deltaImpulse = -jv * effectiveMass;

			F32 maxFriction = constraint.ContactInfo->GetFriction() * constraint.ContactInfo->AccumulatedNormalImpulses[i];
			F32 newImpulse = Math::Clamp(constraint.ContactInfo->AccumulatedTangentImpulses[i] + deltaImpulse, -maxFriction, maxFriction);
			deltaImpulse = newImpulse - constraint.ContactInfo->AccumulatedTangentImpulses[i];
			constraint.ContactInfo->AccumulatedTangentImpulses[i] = newImpulse;

			glm::vec2 impulseVec = deltaImpulse * tangent;

			rbA->SetLinearVelocity(rbA->GetLinearVelocity() + impulseVec * rbA->GetInverseMass());
			rbA->SetAngularVelocity(rbA->GetAngularVelocity() + Math::Cross2D(distA, impulseVec) * rbA->GetInverseInertia());
			rbB->SetLinearVelocity(rbB->GetLinearVelocity() - impulseVec * rbB->GetInverseMass());
			rbB->SetAngularVelocity(rbB->GetAngularVelocity() - Math::Cross2D(distB, impulseVec) * rbB->GetInverseInertia());
		}
	}

	void ContactResolver::ResolveNormalVelocity(const ContactConstraint2D& constraint)
	{
		const ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
		Collider2D* colA = constraint.ContactInfo->Colliders[0];
		Collider2D* colB = constraint.ContactInfo->Colliders[1];
		RigidBody2D* rbA = colA->GetAttachedRigidBody();
		RigidBody2D* rbB = colB->GetAttachedRigidBody();
		
		const Transform2D& tfA = colA->GetAttachedTransform();
		const Transform2D& tfB = colB->GetAttachedTransform();

		for (U32 i = 0; i < manifold.ContactCount; i++)
		{
			glm::vec2 normal = tfA.TransformDirection(manifold.LocalNormal);
			glm::vec2 distA = constraint.DistVecA[i];
			glm::vec2 distB = constraint.DistVecB[i];

			glm::vec2 relativeVel{0.0f};
			relativeVel += rbA->GetLinearVelocity() + Math::Cross2D(rbA->GetAngularVelocity(), distA);
			relativeVel -= rbB->GetLinearVelocity() + Math::Cross2D(rbB->GetAngularVelocity(), distB);
			F32 jv = glm::dot(relativeVel, normal);

			F32 effectiveMass = constraint.NormalMasses[i];

			F32 deltaImpulse = -(jv + constraint.VelocityBias) * effectiveMass;
			F32 newImpulse = Math::Max(constraint.ContactInfo->AccumulatedNormalImpulses[i] + deltaImpulse, 0.0f);
			deltaImpulse = newImpulse - constraint.ContactInfo->AccumulatedNormalImpulses[i];
			constraint.ContactInfo->AccumulatedNormalImpulses[i] = newImpulse;
			
			glm::vec2 impulseVec = deltaImpulse * normal;

			rbA->SetLinearVelocity(rbA->GetLinearVelocity() + impulseVec * rbA->GetInverseMass());
			rbA->SetAngularVelocity(rbA->GetAngularVelocity() + Math::Cross2D(distA, impulseVec) * rbA->GetInverseInertia());
			rbB->SetLinearVelocity(rbB->GetLinearVelocity() - impulseVec * rbB->GetInverseMass());
			rbB->SetAngularVelocity(rbB->GetAngularVelocity() - Math::Cross2D(distB, impulseVec) * rbB->GetInverseInertia());
		}
	}

	void ContactResolver::ResolveVelocity()
	{
		for (auto& constraint : s_ContactConstraints)
		{
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
			const ContactManifold2D& manifold = *constraint.ContactInfo->Manifold;
			Collider2D* colA = constraint.ContactInfo->Colliders[0];
			Collider2D* colB = constraint.ContactInfo->Colliders[1];
			RigidBody2D* rbA = colA->GetAttachedRigidBody();
			RigidBody2D* rbB = colB->GetAttachedRigidBody();
		
			const Transform2D& tfA = colA->GetAttachedTransform();
			const Transform2D& tfB = colB->GetAttachedTransform();
			
			glm::vec2 centerOfMassA = tfA.Transform(rbA->GetCenterOfMass());
			glm::vec2 centerOfMassB = tfB.Transform(rbB->GetCenterOfMass());

			F32 massInvA = rbA->GetInverseMass();
			F32 massInvB = rbB->GetInverseMass();
			F32 inertInvA = rbA->GetInverseInertia();
			F32 inertInvB = rbB->GetInverseInertia();
			
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
				F32 effectiveMass = massInvA + massInvB +
					inertInvA * nDistA * nDistA +
					inertInvB * nDistB * nDistB;

				F32 impulse = effectiveMass > 0.0f ? correction / effectiveMass : 0.0f;
				glm::vec2 impulseVec = impulse * normal;

				rbA->SetPosition(rbA->GetPosition() + impulseVec * rbA->GetInverseMass());
				rbA->AddRotation(rbA->GetInverseInertia() * Math::Cross2D(distA, impulseVec));
				rbB->SetPosition(rbB->GetPosition() - impulseVec * rbB->GetInverseMass());
				rbB->AddRotation(-rbB->GetInverseInertia() * Math::Cross2D(distB, impulseVec));
			}
		}
		return maxDepth < 0.005f * 3.0f;
	}

	void ContactResolver::PostSolve()
	{
		// TODO: put objects to sleep.
	}

	DefaultContactListener* DefaultContactListener::s_Instance = nullptr;

	void DefaultContactListener::Init()
	{
		ENGINE_CORE_ASSERT(s_Instance == nullptr, "Default contact listener is already created.")
		s_Instance = New<DefaultContactListener>();
	}

	void DefaultContactListener::Shutdown()
	{
		if (s_Instance != nullptr)
			Delete<DefaultContactListener>(s_Instance);
	}

	DefaultContactListener* DefaultContactListener::Get()
	{
		if (s_Instance == nullptr)
			Init();
		return s_Instance;
	}	
}