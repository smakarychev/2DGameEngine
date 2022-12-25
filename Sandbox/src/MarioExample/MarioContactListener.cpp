#include "MarioContactListener.h"


void MarioContactListener::OnContactBegin(const Physics::ContactInfo2D& contact)
{
	PropagateToCallback(contact, ContactState::Begin);
	ENGINE_TRACE("On contact begin");
}


void MarioContactListener::OnContactEnd(const Physics::ContactInfo2D& contact)
{
	PropagateToCallback(contact, ContactState::End);
	ENGINE_TRACE("On contact end");
}

void MarioContactListener::PropagateToCallback(const Physics::ContactInfo2D& contact, ContactState state)
{
	Physics::Collider2D* colA = contact.Colliders[0];
	Physics::Collider2D* colB = contact.Colliders[1];

	Entity eA = Entity(static_cast<U32>(reinterpret_cast<U64>(colA->GetUserData())));
	Entity eB = Entity(static_cast<U32>(reinterpret_cast<U64>(colB->GetUserData())));
	
	if (colA->IsSensor() && m_Registry->Has<Component::CollisionCallback>(eA))
	{
		auto& collisionCallback = m_Registry->Get<Component::CollisionCallback>(eA);
		collisionCallback.Callback(m_Registry, {eA, eB, state}, contact);
	}
	else if (colB->IsSensor() && m_Registry->Has<Component::CollisionCallback>(eB))
	{
		auto& collisionCallback = m_Registry->Get<Component::CollisionCallback>(eB);
		collisionCallback.Callback(m_Registry, {eB, eA, state}, contact);
	}
}
