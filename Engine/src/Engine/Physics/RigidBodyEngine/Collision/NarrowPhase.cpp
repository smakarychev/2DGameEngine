#include "enginepch.h"
#include "NarrowPhase.h"

#include "Engine/Core/Core.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	NarrowPhase2D::NarrowPhase2D(BroadPhase2D<>& broadPhase)
		: m_BroadPhase(broadPhase)
	{
	}

	NarrowPhase2D::~NarrowPhase2D()
	{
		ContactInfoNode2D* currentNode = m_ContactInfos;
		while (currentNode != nullptr)
		{
			ContactInfoNode2D* next = currentNode->Next;
			if (currentNode->Info.Manifold != nullptr)
			{
				Delete<ContactManifold2D>(currentNode->Info.Manifold);
			}
			Delete<ContactInfoNode2D>(currentNode);
			currentNode = next;
		}
	}


	void NarrowPhase2D::Collide()
	{
		ContactInfoNode2D* currentInfo = m_ContactInfos;
		while (currentInfo != nullptr)
		{
			// Check if there is still bounds collision.
			const std::array<I32, 2>& broadNodes = currentInfo->Info.NodeIds;
			if (m_BroadPhase.CheckCollision(broadNodes[0], broadNodes[1]) == false)
			{
				m_BroadPhase.RemoveContact(PotentialContact2D{ .Bodies {currentInfo->Info.Bodies}, .NodeIds = broadNodes });
				ContactInfoNode2D* toDelete = currentInfo;
				currentInfo = currentInfo->Next;
				RemoveContactInfo(*toDelete);
				continue;
			}

			Collider2D* colliderA = currentInfo->Info.Bodies[0]->GetCollider();
			Collider2D* colliderB = currentInfo->Info.Bodies[1]->GetCollider();

			Contact2D* narrowContact = ContactManager::Create(colliderA, colliderB);
			if (currentInfo->Info.Manifold == nullptr)
			{
				currentInfo->Info.Manifold = New<ContactManifold2D>();
			}
			if (narrowContact->GenerateContacts(currentInfo->Info) == 0)
			{
				Delete<ContactManifold2D>(currentInfo->Info.Manifold);
				currentInfo->Info.Manifold = nullptr;
			}
			ContactManager::Destroy(narrowContact);
			currentInfo = currentInfo->Next;
		}
	}

	void NarrowPhase2D::Callback(const PotentialContact2D& potentialContact)
	{
		Collider2D* colliderA = potentialContact.Bodies[0]->GetCollider();
		Collider2D* colliderB = potentialContact.Bodies[1]->GetCollider();

		ContactInfo2D info{ .Manifold = nullptr, .Bodies = potentialContact.Bodies, .NodeIds = potentialContact.NodeIds };
		AddContactInfo(info);
	}
	
	ContactInfoNode2D* NarrowPhase2D::AddContactInfo(const ContactInfo2D& info)
	{
		// Allocate new node.
		ContactInfoNode2D* newNode = New<ContactInfoNode2D>();
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
	
	void NarrowPhase2D::RemoveContactInfo(const ContactInfoNode2D& info)
	{
		ContactInfoNode2D* toDelete = const_cast<ContactInfoNode2D*>(&info);
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
		Delete<ContactInfoNode2D>(toDelete);
		m_ContactInfosCount--;
	}
}


