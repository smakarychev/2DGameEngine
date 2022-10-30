#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Core.h"

#include "Engine/ECS/Components.h"

#include <memory>

namespace Engine
{
	using namespace Types;
	// Everything will probably change.
	struct Entity
	{
		friend class EntityManager;
		FRIEND_MEMORY_FN;
		void Destroy() { IsActive = false; }
		
		U64 Id = 0;
		std::string Tag = "Default";
		bool IsActive = true;

		// All possible components (remember, it is testing).
		Ref<Component::Transform2D>		Transform2D;
		Ref<Component::RigidBody2D>		RigidBody2D;
		Ref<Component::BoxCollider2D>	BoxCollider2D;
		Ref<Component::SpriteRenderer>	SpriteRenderer;

		Ref<Component::MarioInput>		MarioInput;

		Ref<Component::GemWarsTransform2D>		GemWarsTransform2D;
		Ref<Component::GemWarsRigidBody2D>		GemWarsRigidBody2D;
		Ref<Component::Mesh2D>					GemWarsMesh2D;
		Ref<Component::GemWarsLifeSpan>			GemWarsLifeSpan;
		Ref<Component::GemWarsInput>			GemWarsInput;
		Ref<Component::GemWarsSpecialAbility>	GemWarsSpecialAbility;
		Ref<Component::GemWarsScore>			GemWarsScore;
		
	private:
		Entity(const std::string& tag, U64 id) :
			Tag(tag), Id(id) {}
	};

}