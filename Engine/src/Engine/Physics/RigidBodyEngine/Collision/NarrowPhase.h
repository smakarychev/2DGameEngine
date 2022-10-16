#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/Collision/BroadPhase.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Contacts.h"

namespace Engine
{
	using namespace Types;

	struct ContactInfoNode2D
	{
		ContactInfo2D Info{};
		ContactInfoNode2D* Next = nullptr;
		ContactInfoNode2D* Prev = nullptr;
	};

	class NarrowPhase2D
	{
	public:
		NarrowPhase2D(BroadPhase2D<>& broadPhase);
		~NarrowPhase2D();

		void Collide();
		const std::vector<ContactManifold2D>& GetContactManifolds() const { return m_Manifolds; }
		void Callback(const PotentialContact2D& potentialContact);
		ContactInfoNode2D* GetContactInfoList() const { return m_ContactInfos; }
		U32 GetContactsCount() const { return m_ContactInfosCount; }
	private:
		ContactInfoNode2D* AddContactInfo(const ContactInfo2D& info);
		void RemoveContactInfo(const ContactInfoNode2D& info);
	private:
		BroadPhase2D<>& m_BroadPhase;
		std::vector<ContactManifold2D> m_Manifolds;

		ContactInfoNode2D* m_ContactInfos = nullptr;
		U32 m_ContactInfosCount = 0;
	};
}