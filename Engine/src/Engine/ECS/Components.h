#pragma once

#include "Engine/Core/Log.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine
{
	struct TransformComponent
	{
		glm::vec3 Position;
		glm::quat Rotation;
		glm::vec3 Scale;
		TransformComponent(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) : 
			Position(pos), Rotation(rot), Scale(scale) {}
	};
}