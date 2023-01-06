#include "MarioContactListener.h"


void MarioContactListener::OnContactBegin(const Physics::ContactInfo2D& contact)
{
	PropagateToCallback(contact, ContactState::Begin);
}


void MarioContactListener::OnContactEnd(const Physics::ContactInfo2D& contact)
{
	PropagateToCallback(contact, ContactState::End);
}

void MarioContactListener::PropagateToCallback(const Physics::ContactInfo2D& contact, ContactState state)
{
	Physics::Collider2D* colA = contact.Colliders[0];
	Physics::Collider2D* colB = contact.Colliders[1];

	Entity eA = Entity(static_cast<U32>(reinterpret_cast<U64>(colA->GetUserData())));
	Entity eB = Entity(static_cast<U32>(reinterpret_cast<U64>(colB->GetUserData())));

	if (SceneUtils::IsDescendant(eA, eB, *m_Registry) || SceneUtils::IsDescendant(eB, eA, *m_Registry)) return;
	
	if (colA->IsSensor() && m_Registry->Has<CollisionCallback>(eA))
	{
		auto& collisionCallback = m_Registry->Get<CollisionCallback>(eA);
		(*m_SensorCallbacks)[collisionCallback.IndexMajor][collisionCallback.IndexMinor](m_Registry, {eA, eB, state}, contact);
	}
	else if (colB->IsSensor() && m_Registry->Has<CollisionCallback>(eB))
	{
		auto& collisionCallback = m_Registry->Get<CollisionCallback>(eB);
		(*m_SensorCallbacks)[collisionCallback.IndexMajor][collisionCallback.IndexMinor](m_Registry, {eB, eA, state}, contact);
	}
}
