#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Physics/NewRBE/RigidBody.h"
#include "Engine/Physics/NewRBE/Collision/BroadPhase.h"
#include "Engine/Physics/NewRBE/Collision/Contacts.h"

namespace Engine::WIP::Physics
{
	using namespace Types;

	class NarrowPhase2D
	{
	public:
		NarrowPhase2D(BroadPhase2D<>& broadPhase);
		~NarrowPhase2D();

		void Clear();
		void DeleteInvalidContacts();
		
		void Collide();
		void OnContactCreate(const PotentialContact2D& potentialContact);
		void OnContactDestroy(ContactInfo2D* contactInfo);
		const ContactInfoList2D& GetContactInfoList() const { return m_ContactList; }
		ContactInfoList2D& GetContactInfoList() { return m_ContactList; }
		U32 GetContactsCount() const { return m_ContactInfosCount; }

		void SetContactListener(ContactListener* contactListener) { m_ContactListener = contactListener; }

	private:
		BroadPhase2D<>& m_BroadPhase;

		ContactListener* m_ContactListener = nullptr;

		ContactInfoList2D m_ContactList;

		NarrowContactAllocator m_ContactAllocator;
		
		U32 m_ContactInfosCount = 0;
	};
}