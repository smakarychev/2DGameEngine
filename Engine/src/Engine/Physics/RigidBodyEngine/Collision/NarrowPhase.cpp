#include "enginepch.h"
#include "NarrowPhase.h"

#include "Engine/Core/Core.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine::Physics
{
	NarrowPhase2D::NarrowPhase2D(BroadPhase2D<>& broadPhase)
		: m_BroadPhase(broadPhase), m_ContactListener(DefaultContactListener::Get())
	{
	}

	NarrowPhase2D::~NarrowPhase2D()
	{
		Clear();
	}

	void NarrowPhase2D::Clear()
	{
		ContactInfoEntry2D* currentNode = m_ContactInfos;
		while (currentNode != nullptr)
		{
			ContactInfoEntry2D* next = currentNode->Next;
			if (currentNode->Info.Manifold != nullptr)
			{
				Delete<ContactManifold2D>(currentNode->Info.Manifold);
			}
			Delete<ContactInfoEntry2D>(currentNode);
			currentNode = next;
		}
		m_ContactInfos = nullptr;
	}


	void NarrowPhase2D::Collide()
	{
		ContactInfoEntry2D* currentInfo = m_ContactInfos;
		while (currentInfo != nullptr)
		{
			// Check if colliders was colliding on the last frame.
			bool wasTouching = currentInfo->Info.IsTouching();
			// Check if there is still bounds collision.
			const std::array<I32, 2>& broadNodes = currentInfo->Info.NodeIds;

			Collider2D* colliderA = currentInfo->Info.Colliders[0];
			Collider2D* colliderB = currentInfo->Info.Colliders[1];
			
			// Check if objects can collide, if we assume object's filters never change,
			// we could move second part of this check to `Callback` and discard contact immediately.
			bool canCollide = m_BroadPhase.CheckCollision(broadNodes[0], broadNodes[1]) &&
				Filter::ShouldCollide(colliderA, colliderB);

			if (canCollide == false)
			{
				if (wasTouching)
				{
					m_ContactListener->OnContactEnd(currentInfo->Info);
					// No need to change state of contact since it will be destroyed immediately.
				}
				m_BroadPhase.RemoveContact(PotentialContact2D{ .Colliders {currentInfo->Info.Colliders}, .NodeIds = broadNodes });
				ContactInfoEntry2D* toDelete = currentInfo;
				currentInfo = currentInfo->Next;
				RemoveContactInfo(*toDelete);
				continue;
			}
			
			Contact2D* narrowContact = ContactManager::Create(colliderA, colliderB);

			if (currentInfo->Info.Manifold == nullptr)
			{
				currentInfo->Info.Manifold = New<ContactManifold2D>();
			}
			bool isTouching = narrowContact->GenerateContacts(currentInfo->Info) > 0;

			if (!isTouching && wasTouching)
			{
				m_ContactListener->OnContactEnd(currentInfo->Info);
				Delete<ContactManifold2D>(currentInfo->Info.Manifold);
				currentInfo->Info.Manifold = nullptr;
				currentInfo->Info.SetTouch(false);
			}
			else if (!wasTouching && isTouching)
			{
 				m_ContactListener->OnContactBegin(currentInfo->Info);
				currentInfo->Info.SetTouch(true);
			}
			ContactManager::Destroy(narrowContact);
			currentInfo = currentInfo->Next;
		}
	}

	void* NarrowPhase2D::OnPotentialContactCreate(const PotentialContact2D& potentialContact)
	{
		ContactInfo2D info{};
		info.Manifold = nullptr;
		info.Colliders = potentialContact.Colliders;
		info.NodeIds = potentialContact.NodeIds;
		info.AccumulatedNormalImpulses = info.AccumulatedTangentImpulses = { 0.0f };
		// Check if any of colliders is sensor, and set flag if so.
		info.SetSensors();
		return AddContactInfo(info);
	}

	void NarrowPhase2D::OnPotentialContactDestroy(ContactInfoEntry2D* contactInfoEntry)
	{
		m_ContactListener->OnContactEnd(contactInfoEntry->Info);
		RemoveContactInfo(*contactInfoEntry);
	}

	ContactInfoEntry2D* NarrowPhase2D::AddContactInfo(const ContactInfo2D& info)
	{
		// Allocate new node.
		ContactInfoEntry2D* newNode = New<ContactInfoEntry2D>();
		newNode->Info = info;
		// Insert node to the list.
		newNode->Next = m_ContactInfos;
		if (m_ContactInfos != nullptr)
		{
			m_ContactInfos->Prev = newNode;
		}
		m_ContactInfos = newNode;
		m_ContactInfosCount++;
		return newNode;
	}
	
	void NarrowPhase2D::RemoveContactInfo(const ContactInfoEntry2D& info)
	{
		ContactInfoEntry2D* toDelete = const_cast<ContactInfoEntry2D*>(&info);
		// Delete node.
		if (toDelete == m_ContactInfos)
		{
			m_ContactInfos = toDelete->Next;
		}
		if (toDelete->Prev != nullptr)
		{
			toDelete->Prev->Next = toDelete->Next;
		}
		if (toDelete->Next != nullptr)
		{
			toDelete->Next->Prev = toDelete->Prev;
		}
		if (toDelete->Info.Manifold != nullptr)
		{
			Delete<ContactManifold2D>(toDelete->Info.Manifold);
		}
		Delete<ContactInfoEntry2D>(toDelete);
		m_ContactInfosCount--;
	}
}