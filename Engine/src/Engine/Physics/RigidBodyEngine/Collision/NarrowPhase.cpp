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
		std::vector<ContactInfo2D> confirmedContacts;
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
			narrowContact->GenerateContacts(confirmedContacts);
			ContactManager::Destroy(narrowContact);

			currentContact = currentContact->Next;
		}
		for (auto& contact : confirmedContacts)
		{
			ContactResolver::Resolve(contact);
			
		}
		/*for (auto& contact : confirmedContacts)
		{
			RigidBody2D* secondBody = contact.Bodies.second;
			secondBody->SetPosition(secondBody->GetPosition() - glm::vec3(contact.Normal * contact.PenetrationDepth, secondBody->GetPosition().z));
		}*/
	}
}


