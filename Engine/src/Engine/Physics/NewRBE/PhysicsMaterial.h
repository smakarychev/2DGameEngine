#pragma once

#include "Engine/Core/Types.h"

namespace Engine::WIP::Physics
{
	using namespace Types;

	// Represents the physical properties of a collider,
	// dictates behaviour during the collision resolution. 
	struct PhysicsMaterial
	{
		F32 Friction;
		F32 Restitution;
		F32 Density;
		PhysicsMaterial(F32 friction = 0.1f, F32 restitution = 0.0f, F32 density = 1.0f)
			: Friction(friction), Restitution(restitution), Density(density)
		{}
	};
}