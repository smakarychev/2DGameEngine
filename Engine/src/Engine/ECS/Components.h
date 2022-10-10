#pragma once

#include "Engine/Core/Log.h"

#include "Engine/Primitives/2D/RegularPolygon.h"
#include "Engine/Rendering/Texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine
{
	using namespace Types;
	namespace Component
	{
		struct Transform
		{
			glm::vec3 Position;
			glm::quat Rotation;
			glm::vec3 Scale;
			Transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) :
				Position(pos), Rotation(rot), Scale(scale) {}
		};

		struct Transform2D
		{
			glm::vec3 Position;
			glm::vec2 Scale;
			F32 Rotation;
			Transform2D(const glm::vec3& pos, const glm::vec2& scale, F32 rotation) :
				Position(pos), Scale(scale), Rotation(rotation)	
			{ }
			Transform2D(const glm::vec2& pos, const glm::vec2& scale, F32 rotation) :
				Position(glm::vec3(pos, 0.0f)), Scale(scale), Rotation(rotation)	
			{ }
		};

		struct RigidBody2D
		{
			F32 CollisionRadius;
			F32 Speed;
			F32 RotationSpeed;
			glm::vec2 Velocity;
			RigidBody2D(F32 colRadius, F32 speed, F32 rotationSpeed = 0.0f) :
				CollisionRadius(colRadius), Speed(speed), RotationSpeed(rotationSpeed), Velocity(glm::vec2(0.0f))
			{ }
		};

		struct Mesh2D
		{
			RegularPolygon Shape;
			glm::vec4 Tint;
			glm::vec2 Tiling;
			std::vector<glm::vec2> UV;
			Texture* Texture;
			//TODO: Most of the parameters one day shall become the part of `Material`.
			Mesh2D(U32 angles, Engine::Texture* texture, const glm::vec4& tint) :
				Shape(angles), Tint(tint), Texture(texture), Tiling(glm::vec2{1.0f}), UV(Shape.GetUVs())
			{ }
			Mesh2D(U32 angles) :
				Shape(angles), Tint(glm::vec4{ 1.0f }), Tiling(glm::vec2{1.0f}), Texture(nullptr), UV(Shape.GetUVs())
			{ }
		};


		/**************** App specific. ***********************/
		struct LifeSpan
		{
			I32 Remaining;
			I32 Total;
			LifeSpan(I32 total) :
				Remaining(total), Total(total)
			{ }
		};

		struct Input
		{
			bool Up, Down, Left, Right;
			bool Shoot, SpecialAbility;
			Input() : 
				Up(false), Down(false), Left(false), Right(false), Shoot(false), SpecialAbility(false)
			{ }
		};

		struct SpecialAbility
		{
			I32 RemainingCoolDown;
			I32 CoolDown;
			SpecialAbility(I32 coolDown) :
				CoolDown(coolDown), RemainingCoolDown(0)
			{ }
		};

		struct Score
		{
			U32 TotalScore;
			Score(U32 score) : TotalScore(score) 
			{ }
		};
	}
	
}