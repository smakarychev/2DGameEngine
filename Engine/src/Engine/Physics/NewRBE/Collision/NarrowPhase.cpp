#include "enginepch.h"
#include "NarrowPhase.h"

#include "Engine/Core/Core.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine::WIP::Physics
{
	NarrowPhase2D::NarrowPhase2D(BroadPhase2D<>& broadPhase)
		: m_BroadPhase(broadPhase), m_ContactListener(DefaultContactListener::Get()), m_ContactAllocator(2_MiB)
	{
	}

	NarrowPhase2D::~NarrowPhase2D()
	{
		Clear();
	}

	void NarrowPhase2D::Clear()
	{
		m_ContactList.Clear();
		m_ContactAllocator.Clear();
	}

	void NarrowPhase2D::DeleteInvalidContacts()
	{
		for (auto it = m_ContactList.begin(); it != m_ContactList.end();)
		{
			ContactInfo2D* contactInfo = it->GetContactInfo();
			++it;
			Collider2D* colliderA = contactInfo->Colliders[0];
			Collider2D* colliderB = contactInfo->Colliders[1];
			if (colliderA->GetAttachedRigidBody() == nullptr || colliderB->GetAttachedRigidBody() == nullptr)
			{
				OnContactDestroy(contactInfo);
			}
		}
	}


	void NarrowPhase2D::Collide()
	{
		m_ContactAllocator.Clear();
		for (auto it = m_ContactList.begin(); it != m_ContactList.end();)
		{
			ContactInfo2D* contactInfo = it->GetContactInfo();
			++it;
			// Check if colliders was colliding on the last frame.
			bool wasTouching = contactInfo->IsTouching();

			// Check if there is still bounds collision.
			Collider2D* colliderA = contactInfo->Colliders[0];
			Collider2D* colliderB = contactInfo->Colliders[1];
			
			// Check if objects can collide, if we assume object's filters never change,
			// we could move second part of this check to `Callback` and discard contact immediately.
			bool canCollide = m_BroadPhase.CheckCollision(colliderA->GetBroadPhaseNode(), colliderB->GetBroadPhaseNode()) &&
				Filter::ShouldCollide(colliderA, colliderB);

			if (canCollide == false)
			{
				if (wasTouching)
				{
					m_ContactListener->OnContactEnd(*contactInfo);
					// No need to change state of contact since it will be destroyed immediately.
				}
				ContactInfo2D* toDelete = contactInfo;
				OnContactDestroy(toDelete);
				continue;
			}
			// Before performing heavy contact generation, check that tight aabb of objects actually collide.
			if (!wasTouching &&
				!colliderA->GenerateBounds(colliderA->GetAttachedTransform())
					.Intersects(colliderB->GenerateBounds(colliderB->GetAttachedTransform()))) continue;
			Contact2D* narrowContact = ContactManager::Create(m_ContactAllocator, colliderA, colliderB);
			if (contactInfo->Manifold == nullptr)
			{
				contactInfo->Manifold = New<ContactManifold2D>();
			}
			bool isTouching = narrowContact->GenerateContacts(*contactInfo) > 0;

			if (!isTouching && wasTouching)
			{
				m_ContactListener->OnContactEnd(*contactInfo);
				Delete<ContactManifold2D>(contactInfo->Manifold);
				contactInfo->Manifold = nullptr;
				contactInfo->SetTouch(false);
			}
			else if (!wasTouching && isTouching)
			{
				m_ContactListener->OnContactBegin(*contactInfo);
				contactInfo->SetTouch(true);
			}
		}

	}

	void NarrowPhase2D::OnContactCreate(const PotentialContact2D& potentialContact)
	{
		m_ContactList.Push(potentialContact);
	}

	void NarrowPhase2D::OnContactDestroy(ContactInfo2D* contactInfo)
	{
		m_BroadPhase.RemoveContact(PotentialContact2D{ .Colliders {contactInfo->Colliders} });
		m_ContactList.Pop(contactInfo);
	}
	
}