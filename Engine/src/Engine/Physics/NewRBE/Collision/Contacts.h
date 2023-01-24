#pragma once

#include "Collider.h"

namespace Engine::WIP::Physics
{
	// Represents intersection info of pair of 2 rigidbodies,
	// after resolution those bodies are no longer intersecting,
	// sufficient impulses applied.
	struct ContactPoint2D
	{
		glm::vec2 LocalPoint;
		F32 PenetrationDepth;
	};

	struct ContactManifold2D
	{
		// There are no more than 2 intersection points in 2d.
		std::array<ContactPoint2D, 2> Contacts;
		glm::vec2 LocalReferencePoint;
		U32 ContactCount;
		glm::vec2 LocalNormal;
	};

	struct ContactInfo2D
	{
		ContactManifold2D* Manifold = nullptr;
		std::array<Collider2D*, 2> Colliders{};
		ContactInfoEntry2D* ContactInfoEntry{nullptr};
		std::array<F32, 2> AccumulatedNormalImpulses { 0.0f };
		std::array<F32, 2> AccumulatedTangentImpulses { 0.0f };
		void SetTouch(bool isTouching)
		{
			if (isTouching) Flags |= TOUCH_FLAG;
			else Flags &= ~TOUCH_FLAG;
		}
		bool IsTouching() const { return static_cast<bool>(Flags & TOUCH_FLAG); }
		void SetSensors()
		{
			ENGINE_CORE_ASSERT(Colliders[0] != nullptr && Colliders[1] != nullptr, "Colliders are unset")
			if (Colliders[0]->IsSensor() || Colliders[1]->IsSensor())
			{
				Flags |= SENSOR_FLAG;
			}
			else
			{
				Flags &= ~SENSOR_FLAG;
			}
		}
		bool HasSensors() const { return static_cast<bool>(Flags & SENSOR_FLAG); }
		F32 GetRestitution() const
		{
			return std::max(
				Colliders[0]->GetPhysicsMaterial().Restitution,
				Colliders[1]->GetPhysicsMaterial().Restitution);
		}
		F32 GetFriction() const
		{
			return std::min(
				Colliders[0]->GetPhysicsMaterial().Friction,
				Colliders[1]->GetPhysicsMaterial().Friction);
		}
	private:
		U32 Flags{0};
		constexpr static auto TOUCH_FLAG{Bit(0)};
		constexpr static auto SENSOR_FLAG{Bit(1)};
	};

	struct ContactConstraint2D
	{
		ContactInfo2D* ContactInfo{nullptr};
		std::array<F32, 2> NormalMasses{};
		std::array<F32, 2> TangentMasses{};
		std::array<glm::vec2, 2> DistVecA{};
		std::array<glm::vec2, 2> DistVecB{};
		// We have to calculate bias from restitution only once.
		F32 VelocityBias{0.0f};
	};

	class Contact2D
	{
		friend class ContactManager;
	public:
		virtual ~Contact2D() {}
		// Populates vector of contacts, returns the amount of added contacts.
		virtual U32 GenerateContacts(ContactInfo2D& info) = 0;
		virtual std::array<Collider2D*, 2> GetColliders() = 0;
	private:
		// Shall never be called (it means we failed to derive real type).
		static Contact2D* Create(NarrowContactAllocator& alloc, Collider2D* first, Collider2D* second) { ENGINE_CORE_ASSERT(false, "Failed to derive type.") return nullptr; }
	};

	class PolygonPolygonContact2D : public Contact2D
	{
		friend class ContactManager;
	public:
		PolygonPolygonContact2D(PolygonCollider2D* first, PolygonCollider2D* second);

		U32 GenerateContacts(ContactInfo2D& info) override;
		std::array<Collider2D*, 2> GetColliders() override { return { m_First, m_Second }; }
	private:
		static Contact2D* Create(NarrowContactAllocator& alloc, Collider2D* a, Collider2D* b);
	private:
		PolygonCollider2D* m_First, * m_Second;
	};

	class CircleCircleContact2D : public Contact2D
	{
		friend class ContactManager;
	public:
		CircleCircleContact2D(CircleCollider2D* first, CircleCollider2D* second);

		U32 GenerateContacts(ContactInfo2D& info)   override;
		std::array<Collider2D*, 2> GetColliders() override { return { m_First, m_Second }; }
	private:
		static Contact2D* Create(NarrowContactAllocator& alloc, Collider2D* a, Collider2D* b);
	private:
		CircleCollider2D* m_First, * m_Second;
	};

	// TODO: true edge (currently acts like a plane).
	class EdgeCircleContact2D : public Contact2D
	{
		friend class ContactManager;
	public:
		EdgeCircleContact2D(EdgeCollider2D* edge, CircleCollider2D* circle);

		U32 GenerateContacts(ContactInfo2D& info) override;
		std::array<Collider2D*, 2> GetColliders() override { return { m_Edge, m_Circle }; }
	private:
		static Contact2D* Create(NarrowContactAllocator& alloc, Collider2D* a, Collider2D* b);
	private:
		EdgeCollider2D* m_Edge;
		CircleCollider2D* m_Circle;
	};

	struct ContactRegistration
	{
		using OnCreateFn = Contact2D* (*)(NarrowContactAllocator& alloc, Collider2D*, Collider2D*);
		OnCreateFn CreateFn;
		// So we need twice as less methods.
		bool IsPrimary;
	};

	class ContactManager
	{
	public:
		static void Init();
		static void AddRegistration(
			ContactRegistration::OnCreateFn createFn,
			Collider2D::Type typeA,
			Collider2D::Type typeB
		);
		static Contact2D* Create(NarrowContactAllocator& alloc, Collider2D* a, Collider2D* b);

		static ContactRegistration s_Registry
			[static_cast<U32>(Collider2D::Type::TypesCount)]
			[static_cast<U32>(Collider2D::Type::TypesCount)];

		static bool s_IsInit;
	};

	struct ContactInfoEntry2D;

	struct ContactResolverDef
	{
		ContactInfoList2D* ContactList{nullptr};
		U32 ContactListSize = 0;
		bool WarmStartEnabled = false;
	};

	/*
	 * TODO (possible optimization):
	 **** Store positions and velocities in constraint itself, and then reassign it to the body.
	 */
	class ContactResolver
	{
	public:
		static void PreSolve(const ContactResolverDef& crDef);
		static void WarmStart();
		static void ResolveVelocity();
		static bool ResolvePosition();
		// Put bodies to sleep.
		static void PostSolve();

		static void StoreContactPoints(bool store) { s_StoreContactPointInfo = store; }
		static const std::vector<glm::vec2>& GetContactPoints() { return s_ContactPoints; }
		static const std::vector<glm::vec2>& GetContactNormals() { return s_ContactNormals; }
	private:
		static void ResolveTangentVelocity(const ContactConstraint2D& constraint);
		static void ResolveNormalVelocity(const ContactConstraint2D& constraint);
	private:
		static std::vector<ContactConstraint2D> s_ContactConstraints;
		static std::vector<glm::vec2> s_ContactPoints;
		static std::vector<glm::vec2> s_ContactNormals;
		static bool s_StoreContactPointInfo;
	};

	// Idea from box2d (as pretty much everything else here :)),
	// let user react to contact's state change.
	class ContactListener
	{
	public:
		enum class ContactState { Begin, End };
	public:
		virtual ~ContactListener() {}
		virtual void OnContactBegin([[maybe_unused]] const ContactInfo2D& contact) {}
		virtual void OnContactEnd([[maybe_unused]] const ContactInfo2D& contact) {}
	};

	class DefaultContactListener : public ContactListener
	{
	public:
		static void Init();
		// This is called in `EntryPoint` maybe find a better place?
		static void Shutdown();
		static DefaultContactListener* Get();
	private:
		static DefaultContactListener* s_Instance;
	};
}