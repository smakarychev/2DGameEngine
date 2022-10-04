#pragma once

#include "Engine/Core/Types.h"

namespace Engine
{
	using namespace Types;

	// Represents the physical properties of a rigid body,
	// dictates behaviour during the collision resolution. 
	struct PhysicsMaterial
	{
		F32 Friction;
		F32 Restitution;
		PhysicsMaterial(F32 friction = 0.1f, F32 restituion = 0.0f)
			: Friction(friction), Restitution(restituion)
		{}
	};

}
