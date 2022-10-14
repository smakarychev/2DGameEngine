#include "enginepch.h"
#include "NarrowPhase.h"

#include "Engine/Core/Core.h"

namespace Engine
{
	NarrowPhase2D::NarrowPhase2D(BroadPhase2D<>& broadPhase)
		: m_BroadPhase(broadPhase)
	{
	}

	void NarrowPhase2D::Collide(const PotentialContactNode2D* potentialContacts)
	{
			std::vector<ContactManifold2D> contactManifolds;
			const PotentialContactNode2D* currentContact = potentialContacts;
			while (currentContact != nullptr)
			{
				// Check if there is still bounds collision.
				if (m_BroadPhase.CheckCollision(currentContact->Contact.NodeIds[0], currentContact->Contact.NodeIds[1]) == false)
				{
					m_BroadPhase.RemoveContact(currentContact->Contact);
					currentContact = currentContact->Next;
					continue;
				}

				Collider2D* colliderA = currentContact->Contact.Bodies[0]->GetCollider();
				Collider2D* colliderB = currentContact->Contact.Bodies[1]->GetCollider();

				Contact2D* narrowContact = ContactManager::Create(colliderA, colliderB);
				U32 foundContacts = narrowContact->GenerateContacts(contactManifolds);
				ContactManager::Destroy(narrowContact);

				currentContact = currentContact->Next;
			}
			//TODO: temp - construct constrains.
			/*std::vector<ContactConstraint2D> constrains;
			constrains.reserve(contactManifolds.size());
			for (auto& manifold : contactManifolds)
			{
				constrains.push_back({ .Manifold = &manifold, .AccumulatedImpulse = 0.0f });
			}*/
			for (auto& manifold : contactManifolds)
			{
				ContactResolver::Resolve(manifold);
			}
	}
}


