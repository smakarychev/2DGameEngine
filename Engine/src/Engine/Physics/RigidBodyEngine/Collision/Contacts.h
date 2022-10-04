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
		struct BodiesInfo
		{
			RigidBody2D* first;
			RigidBody2D* second;
		};
		glm::vec2 Point;
		glm::vec2 Normal;
		F32 PenetrationDepth;
	};

	class Contact2D
	{
	public:
		// Shall never be called (it means we failed to derive real type).
		static Contact2D* Create(Collider2D* first, Collider2D* second) { return nullptr; }
		static void Destroy(Contact2D* contact) {}
		// Populates vector of contacts, returns the amount of added contacts.
		virtual U32 GenerateContacts(std::vector<ContactInfo2D>& contacts) = 0;
	};

	class BoxBoxContact2D : public Contact2D
	{
	public:
		BoxBoxContact2D(BoxCollider2D* first, BoxCollider2D* second);
			
		static Contact2D* Create(Collider2D* a, Collider2D* b);
		
		static void Destroy(Contact2D* contact);
		
		U32 GenerateContacts(std::vector<ContactInfo2D>& contacts) override;

	private:
		BoxCollider2D* m_First, * m_Second;
	};

	class CircleCircleContact2D : public Contact2D
	{
	public:
		CircleCircleContact2D(CircleCollider2D* first, CircleCollider2D* second);

		static Contact2D* Create(Collider2D* a, Collider2D* b);

		static void Destroy(Contact2D* contact);

		U32 GenerateContacts(std::vector<ContactInfo2D>& contacts) override;

	private:
		CircleCollider2D* m_First, * m_Second;
	};

	class BoxCircleContact2D : public Contact2D
	{
	public:
		BoxCircleContact2D(BoxCollider2D* box, CircleCollider2D* circle);

		static Contact2D* Create(Collider2D* a, Collider2D* b);

		static void Destroy(Contact2D* contact);

		U32 GenerateContacts(std::vector<ContactInfo2D>& contacts) override;

	private:
		BoxCollider2D* m_Box;
		CircleCollider2D* m_Circle;
	};

	// TODO: true edge (currently acts like a plane).
	class EdgeCircleContact2D : public Contact2D
	{
	public:
		EdgeCircleContact2D(EdgeCollider2D* edge, CircleCollider2D* circle);

		static Contact2D* Create(Collider2D* a, Collider2D* b);

		static void Destroy(Contact2D* contact);

		U32 GenerateContacts(std::vector<ContactInfo2D>& contacts) override;

	private:
		EdgeCollider2D* m_Edge;
		CircleCollider2D* m_Circle;
	};

	// TODO: true edge (currently acts like a plane).
	class EdgeBoxContact2D : public Contact2D
	{
	public:
		EdgeBoxContact2D(EdgeCollider2D* edge, BoxCollider2D* box);

		static Contact2D* Create(Collider2D* a, Collider2D* b);

		static void Destroy(Contact2D* contact);

		U32 GenerateContacts(std::vector<ContactInfo2D>& contacts) override;

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

		static ContactRegistation s_Registry
			[static_cast<U32>(Collider2D::Type::TypesCount)]
			[static_cast<U32>(Collider2D::Type::TypesCount)];

	};

}