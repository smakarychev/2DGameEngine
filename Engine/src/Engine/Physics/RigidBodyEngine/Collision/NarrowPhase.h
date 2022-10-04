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