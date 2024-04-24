#pragma once
#include <glm/vec2.hpp>

#include "Engine/Core/Types.h"
#include "Engine/Memory/StackAllocator.h"
#include "Engine/Physics/NewRBE/Newest/Collision/BroadPhase/BroadPhase.h"

namespace Engine::WIP::Physics::Newest
{
	using namespace Types;
	
    // Represents intersection info of pair of 2 rigidbodies,
	// after resolution those bodies are no longer intersecting,
	// sufficient impulses applied.
	struct ContactPoint2D
	{
		glm::vec2 LocalPoint;
		F32 PenetrationDepth;
	};

	struct ContactManifold2D
	{
		// There are no more than 2 intersection points in 2d.
		std::array<ContactPoint2D, 2> Contacts;
		glm::vec2 LocalReferencePoint;
		U32 ContactCount;
		glm::vec2 LocalNormal;
	};

	struct ContactInfo2D
	{
		ContactManifold2D Manifold{};
		BroadContactPair ContactPair{};
		std::array<F32, 2> AccumulatedNormalImpulses{0.0f};
		std::array<F32, 2> AccumulatedTangentImpulses{0.0f};
	};

	struct ContactConstraint2D
	{
		ContactInfo2D* ContactInfo{nullptr};
		std::array<F32, 2> NormalMasses{};
		std::array<F32, 2> TangentMasses{};
		std::array<glm::vec2, 2> DistVecA{};
		std::array<glm::vec2, 2> DistVecB{};
		// We have to calculate bias from restitution only once.
		F32 VelocityBias{0.0f};
	};

	using ContactAllocator = StackAllocator;
	
	class Contact2D
	{
		friend class ContactManager;
	public:
		virtual ~Contact2D() {}
		// Populates vector of contacts, returns the amount of added contacts.
		virtual U32 GenerateContacts(ContactInfo2D& info) = 0;
		virtual std::array<Collider2D*, 2> GetColliders() = 0;
	private:
		// Shall never be called (it means we failed to derive real type).
		static Contact2D* Create(ContactAllocator& alloc, Collider2D* first, Collider2D* second) { return nullptr; }
		static void Destroy(Contact2D* contact) {}
	};
}
