#include "enginepch.h"
#include "NarrowPhase.h"

#include "Engine/Core/Core.h"

namespace Engine
{
	NarrowPhase2D::NarrowPhase2D(BroadPhase2D<>& broadPhase)
		: m_BroadPhase(broadPhase)
	{
	}

	void NarrowPhase2D::Collide(const std::vector<PotentialContact2D>& potentialContacts)
	{
		std::vector<ContactInfo2D> confirmedContacts;
		for (auto& contact : potentialContacts)
		{
			// Check if there is still bounds collision.
			if (m_BroadPhase.CheckCollision(contact.NodeIds[0], contact.NodeIds[1]) == false)
			{
				m_BroadPhase.RemoveContact(contact);
				continue;
			}

			Collider2D* colliderA = contact.Bodies[0]->GetCollider();
			Collider2D* colliderB = contact.Bodies[1]->GetCollider();

			Contact2D* narrowContact = ContactManager::Create(colliderA, colliderB);
			narrowContact->GenerateContacts(confirmedContacts);		
		}
		/*for (auto& contact : confirmedContacts)
		{
			RigidBody2D* secondBody = contact.Bodies.second;
			secondBody->SetPosition(secondBody->GetPosition() - glm::vec3(contact.Normal * contact.PenetrationDepth, secondBody->GetPosition().z));
		}*/
	}
}


