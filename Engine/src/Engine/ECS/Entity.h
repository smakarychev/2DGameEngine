#pragma once

#include "Engine/Core/Types.h"

#include "Engine/ECS/Components.h"

#include <memory>

namespace Engine
{
	// Everything will probably change.
	struct Entity
	{
		Entity(const std::string& tag, U64 id) :
			Tag(tag), Id(id) {}

		void Destroy() { IsAlive = false; }
		
		U64 Id = 0;
		std::string Tag = "Default";
		bool IsAlive = true;

		// All possible components (remember, it is testing).
		std::shared_ptr<TransformComponent> Transform;
	};

}