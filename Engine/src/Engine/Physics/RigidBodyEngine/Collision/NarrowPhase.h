#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"

namespace Engine
{
	using namespace Types;

	// Represents intersection info of pair of 2 rigidbodies,
	// after resolution those bodies are no longer intersecting,
	// sufficient impulses applied.
	class ContactInfo2D
	{
		glm::vec3 Point;
		glm::vec2 Normal;
		F32 PenetrationDepth;
	};

	class NarrowPhase2D
	{};
}


// TODO: testy-shmesty-tempsty
// not a valid code.
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Memory/MemoryUtils.h"
#include "Engine/Math/MathUtils.h"
namespace Engine
{
namespace TEMP
{
	class PhysicsMaterial {};
	class Collider 
	{
	public:
		enum class Type
		{
			Box = 0, Circle = 1, TypesCount = 2
		};
		Type mType;
		Collider(Type type) : mType(type) {}
	};
	class BoxCollider : public Collider 
	{
	public:
		BoxCollider() : Collider(Type::Box) {}
	};
	class CircleCollider : public Collider
	{
	public:
		CircleCollider() : Collider(Type::Circle) {}
	};

	class Contact 
	{
	public:
		static Contact* Create(Collider* a, Collider* b) { return nullptr; }
		static void Destoy(Contact* contact) {}
	};
	class BoxBoxContact : public Contact 
	{
	public:
		static Contact* Create(Collider* a, Collider* b)
		{
			return New<BoxBoxContact>();
		}
		static void Destoy(Contact* contact)
		{
			Delete<BoxBoxContact>(reinterpret_cast<BoxBoxContact*>(contact));
		}
	};
	class CircleCircleContact : public Contact {};
	class BoxCircleContact : public Contact {};

	struct ContactRegistation
	{
		using OnCreateFn = Contact * (*)(Collider*, Collider*);
		using OnDestroyFn = void (*)(Contact*);
		OnCreateFn CreateFn;
		OnDestroyFn DestroyFn;
		// So we need twice as less CreateFunctions.
		bool IsPrimary;
	};


	class ContactManager
	{
	public:
		static void Init();
		static void AddRegistration(
			ContactRegistation::OnCreateFn createFn,
			ContactRegistation::OnDestroyFn destroyFn,
			Collider::Type typeA,
			Collider::Type typeB
		);
		static Contact* Create(Collider* a, Collider* b);

		static ContactRegistation s_Registry
			[static_cast<U32>(Collider::Type::TypesCount)]
			[static_cast<U32>(Collider::Type::TypesCount)];
		

	};
	ContactRegistation ContactManager::s_Registry
		[static_cast<U32>(Collider::Type::TypesCount)]
		[static_cast<U32>(Collider::Type::TypesCount)];

	Contact* ContactManager::Create(Collider* a, Collider* b)
	{
		Collider::Type typeA = a->mType;
		Collider::Type typeB = b->mType;
		U32 typeAI = static_cast<U32>(typeA);
		U32 typeBI = static_cast<U32>(typeB);
		return s_Registry[typeAI, typeBI]->CreateFn(a, b);
	}

	// Create matrix of dipatch functions.
	void ContactManager::Init()
	{
		AddRegistration(BoxBoxContact::Create,		 BoxBoxContact::Destoy,		  Collider::Type::Box,	  Collider::Type::Box);
		AddRegistration(BoxCircleContact::Create,	 BoxCircleContact::Destoy,	  Collider::Type::Box,	  Collider::Type::Circle);
		AddRegistration(CircleCircleContact::Create, CircleCircleContact::Destoy, Collider::Type::Circle, Collider::Type::Circle);
	}

	void ContactManager::AddRegistration(
		ContactRegistation::OnCreateFn createFn,
		ContactRegistation::OnDestroyFn destroyFn,
		Collider::Type typeA,
		Collider::Type typeB
	)
	{
		U32 typeAI = static_cast<U32>(typeA);
		U32 typeBI = static_cast<U32>(typeB);
		s_Registry[typeAI][typeBI].CreateFn = createFn;
		s_Registry[typeAI][typeBI].DestroyFn = destroyFn;
		s_Registry[typeAI][typeBI].IsPrimary = true;

		if (typeAI != typeBI)
		{
			s_Registry[typeBI][typeAI].CreateFn = createFn;
			s_Registry[typeBI][typeAI].DestroyFn = destroyFn;
			s_Registry[typeBI][typeAI].IsPrimary = false;
		}
	}

	class Body
	{
		PhysicsMaterial mat;
		Collider* col;
	};

	class World
	{
		std::vector<Body> bodies;

	};
}
}