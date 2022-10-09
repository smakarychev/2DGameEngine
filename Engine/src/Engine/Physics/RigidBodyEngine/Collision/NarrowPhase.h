#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Physics/RigidBodyEngine/Collision/BroadPhase.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Contacts.h"

namespace Engine
{
	using namespace Types;

	// TODO: abandon narrow phase for contact manager (basically just a name change)?
	class NarrowPhase2D
	{
	public:
		NarrowPhase2D(BroadPhase2D<>& broadPhase);
		void Collide(const PotentialContactNode2D* potentialContacts);
	private:
		BroadPhase2D<>& m_BroadPhase;
	};
}