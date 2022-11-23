#include "MarioContactListener.h"


void MarioContactListener::OnContactBegin(const Physics::ContactInfo2D& contact)
{
	Physics::Collider2D* colA = contact.Colliders[0];
	Physics::Collider2D* colB = contact.Colliders[1];
	Physics::RigidBody2D* bodyA = const_cast<Physics::RigidBody2D*>(colA->GetAttachedRigidBody());
	Physics::RigidBody2D* bodyB = const_cast<Physics::RigidBody2D*>(colB->GetAttachedRigidBody());

	if (colA->IsSensor())
	{
		MarioGameSensorCallback callback = (MarioGameSensorCallback)colA->GetUserData();
		callback(m_Registry, bodyA->GetUserData(), ContactListener::ContactState::Begin, contact);
	}
	else if (colB->IsSensor())
	{
		MarioGameSensorCallback callback = (MarioGameSensorCallback)colB->GetUserData();
		callback(m_Registry, bodyB->GetUserData(), ContactListener::ContactState::Begin, contact);
	}
}


void MarioContactListener::OnContactEnd(const Physics::ContactInfo2D& contact)
{
	Physics::Collider2D* colA = contact.Colliders[0];
	Physics::Collider2D* colB = contact.Colliders[1];
	Physics::RigidBody2D* bodyA = const_cast<Physics::RigidBody2D*>(colA->GetAttachedRigidBody());
	Physics::RigidBody2D* bodyB = const_cast<Physics::RigidBody2D*>(colB->GetAttachedRigidBody());

	if (colA->IsSensor())
	{
		MarioGameSensorCallback callback = (MarioGameSensorCallback)colA->GetUserData();
		callback(m_Registry, bodyA->GetUserData(), ContactListener::ContactState::End, contact);
	}
	else if (colB->IsSensor())
	{
		MarioGameSensorCallback callback = (MarioGameSensorCallback)colB->GetUserData();
		callback(m_Registry, bodyB->GetUserData(), ContactListener::ContactState::End, contact);
	}
}
