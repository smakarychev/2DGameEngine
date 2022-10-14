#pragma once

#include "Collider.h"

#include "Engine/Physics/RigidBodyEngine/RigidBody.h"

namespace Engine
{
	// Represents intersection info of pair of 2 rigidbodies,
	// after resolution those bodies are no longer intersecting,
	// sufficient impulses applied.
	struct ContactInfo2D
	{
		glm::vec2 Point;
		F32 PenetrationDepth;
	};

	struct ContactManifold2D
	{
		// There are no more than 2 intersection points in 2d.
		std::array<ContactInfo2D, 2> Contacts;
		U32 ContactCount;
		glm::vec2 ContactNormal;
		std::array<RigidBody2D*, 2> Bodies;
		F32 GetRestitution() const
		{
			return std::max(
				Bodies[0]->GetPhysicsMaterial().Restitution,
				Bodies[1]->GetPhysicsMaterial().Restitution);
		}
	};

	struct ContactConstraint2D
	{
		ContactManifold2D* Manifold;
		F32 AccumulatedImpulse;
	};

	class Contact2D
	{
		friend class ContactManager;
	public:
		// Populates vector of contacts, returns the amount of added contacts.
		virtual U32 GenerateContacts(std::vector<ContactManifold2D>& manifolds) = 0;
		virtual std::array<Collider2D*, 2> GetColliders() = 0;
	private:
		// Shall never be called (it means we failed to derive real type).
		static Contact2D* Create(Collider2D* first, Collider2D* second) { return nullptr; }
		static void Destroy(Contact2D* contact) {}
	};

	class BoxBoxContact2D : public Contact2D
	{
		friend class ContactManager;
	public:
		BoxBoxContact2D(BoxCollider2D* first, BoxCollider2D* second);
			
		U32 GenerateContacts(std::vector<ContactManifold2D>& manifolds) override;
		std::array<Collider2D*, 2> GetColliders() override { return { m_First, m_Second }; }
	private:
		static Contact2D* Create(Collider2D* a, Collider2D* b);
		static void Destroy(Contact2D* contact);

	private:
		BoxCollider2D* m_First, * m_Second;
	};

	class CircleCircleContact2D : public Contact2D
	{
		friend class ContactManager;
	public:
		CircleCircleContact2D(CircleCollider2D* first, CircleCollider2D* second);

		U32 GenerateContacts(std::vector<ContactManifold2D>& manifolds) override;
		std::array<Collider2D*, 2> GetColliders() override { return { m_First, m_Second }; }
	private:
		static Contact2D* Create(Collider2D* a, Collider2D* b);
		static void Destroy(Contact2D* contact);

	private:
		CircleCollider2D* m_First, * m_Second;
	};

	class BoxCircleContact2D : public Contact2D
	{
		friend class ContactManager;
	public:
		BoxCircleContact2D(BoxCollider2D* box, CircleCollider2D* circle);

		U32 GenerateContacts(std::vector<ContactManifold2D>& manifolds) override;
		std::array<Collider2D*, 2> GetColliders() override { return { m_Box, m_Circle }; }
	private:
		static Contact2D* Create(Collider2D* a, Collider2D* b);
		static void Destroy(Contact2D* contact);

	private:
		BoxCollider2D* m_Box;
		CircleCollider2D* m_Circle;
	};

	// TODO: true edge (currently acts like a plane).
	class EdgeCircleContact2D : public Contact2D
	{
		friend class ContactManager;
	public:
		EdgeCircleContact2D(EdgeCollider2D* edge, CircleCollider2D* circle);

		U32 GenerateContacts(std::vector<ContactManifold2D>& manifolds) override;
		std::array<Collider2D*, 2> GetColliders() override { return { m_Edge, m_Circle }; }
	private:
		static Contact2D* Create(Collider2D* a, Collider2D* b);
		static void Destroy(Contact2D* contact);
	private:
		EdgeCollider2D* m_Edge;
		CircleCollider2D* m_Circle;
	};

	// TODO: true edge (currently acts like a plane).
	class EdgeBoxContact2D : public Contact2D
	{
		friend class ContactManager;
	public:
		EdgeBoxContact2D(EdgeCollider2D* edge, BoxCollider2D* box);

		U32 GenerateContacts(std::vector<ContactManifold2D>& manifolds) override;
		std::array<Collider2D*, 2> GetColliders() override { return { m_Edge, m_Box }; }
	private:
		static Contact2D* Create(Collider2D* a, Collider2D* b);
		static void Destroy(Contact2D* contact);
	private:
		EdgeCollider2D* m_Edge;
		BoxCollider2D* m_Box;
	};

	struct ContactRegistation
	{
		using OnCreateFn = Contact2D* (*)(Collider2D*, Collider2D*);
		using OnDestroyFn = void (*)(Contact2D*);
		OnCreateFn CreateFn;
		OnDestroyFn DestroyFn;
		// So we need twice as less methods.
		bool IsPrimary;
	};

	class ContactManager
	{
	public:
		static void Init();
		static void AddRegistration(
			ContactRegistation::OnCreateFn createFn,
			ContactRegistation::OnDestroyFn destroyFn,
			Collider2D::Type typeA,
			Collider2D::Type typeB
		);
		static Contact2D* Create(Collider2D* a, Collider2D* b);
		static void Destroy(Contact2D* contact);

		static ContactRegistation s_Registry
			[static_cast<U32>(Collider2D::Type::TypesCount)]
			[static_cast<U32>(Collider2D::Type::TypesCount)];

		static bool s_IsInit;
	};

	class ContactResolver
	{
	public:
		static void Resolve(const ContactManifold2D& manifold);
	};
}