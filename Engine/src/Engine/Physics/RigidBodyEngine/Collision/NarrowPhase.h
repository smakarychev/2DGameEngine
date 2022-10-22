#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/Collision/BroadPhase.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Contacts.h"

namespace Engine
{
	using namespace Types;

	struct ContactInfoEntry2D
	{
		ContactInfo2D Info{};
		ContactInfoEntry2D* Next = nullptr;
		ContactInfoEntry2D* Prev = nullptr;
	};

	class NarrowPhase2D
	{
	public:
		NarrowPhase2D(BroadPhase2D<>& broadPhase);
		~NarrowPhase2D();

		void Collide();
		void Callback(const PotentialContact2D& potentialContact);
		ContactInfoEntry2D* GetContactInfoList() const { return m_ContactInfos; }
		U32 GetContactsCount() const { return m_ContactInfosCount; }

		void SetContactListener(ContactListener* contactListener) { m_ContactListener = contactListener; }

	private:
		ContactInfoEntry2D* AddContactInfo(const ContactInfo2D& info);
		void RemoveContactInfo(const ContactInfoEntry2D& info);
	private:
		BroadPhase2D<>& m_BroadPhase;

		ContactListener* m_ContactListener = nullptr;

		ContactInfoEntry2D* m_ContactInfos = nullptr;
		U32 m_ContactInfosCount = 0;
	};
}