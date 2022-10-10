#pragma once

#include "Engine/Core/Types.h"

#include "Engine/ECS/Components.h"

#include <memory>

namespace Engine
{
	using namespace Types;
	// Everything will probably change.
	struct Entity
	{
		Entity(const std::string& tag, U64 id) :
			Tag(tag), Id(id) {}

		void Destroy() { IsActive = false; }
		
		U64 Id = 0;
		std::string Tag = "Default";
		bool IsActive = true;

		// All possible components (remember, it is testing).
		std::shared_ptr<Component::Transform2D>		Transform2D;
		std::shared_ptr<Component::RigidBody2D>		RigidBody2D;
		std::shared_ptr<Component::Mesh2D>			Mesh2D;
		std::shared_ptr<Component::LifeSpan>		LifeSpan;
		std::shared_ptr<Component::Input>			Input;
		std::shared_ptr<Component::SpecialAbility>	SpecialAbility;
		std::shared_ptr<Component::Score>			Score;
	};

}