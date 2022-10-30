#pragma once

#include "Engine/Core/Log.h"

#include "Engine/Primitives/2D/RegularPolygon.h"
#include "Engine/Rendering/Texture.h"
#include "Engine/Rendering/SortingLayer.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"

#include "Engine/Memory/MemoryManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

#include "Engine/Rendering/Font.h"
#include "Engine/Rendering/SortingKey.h"

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
			struct Rotation
			{
				glm::vec2 RotationVec;
				Rotation(const glm::vec2& rotation) : RotationVec(rotation) {}
				Rotation(F32 angleRad) : RotationVec(glm::cos(angleRad), glm::sin(angleRad)) {}
				operator const glm::vec2&() const { return RotationVec; }
				operator glm::vec2&() { return RotationVec; }
				F32& operator[](I32 index) { return RotationVec[index]; }
				const F32& operator[](I32 index) const { return RotationVec[index]; }
			};
			glm::vec2 Position	= glm::vec2{0.0f};
			glm::vec2 Scale		= glm::vec2{1.0f};
			Rotation Rotation	= glm::vec2{1.0f, 0.0f};
			Transform2D(const glm::vec2& pos, const glm::vec2& scale, const glm::vec2& rotation) :
				Position(pos), Scale(scale), Rotation(rotation) {}
			Transform2D(const glm::vec2& pos, const glm::vec2& scale, F32 rotation) :
				Position(pos), Scale(scale), Rotation(rotation) {}
			Transform2D() = default;
		};

		using PhysicsMaterial = Engine::PhysicsMaterial;

		// Stores lightweight representation of physics engine version,
		// Simplifies creation process.
		struct RigidBody2D
		{
			// Pointer to real (physics engine's) body.
			Engine::RigidBody2D* PhysicsBody	= nullptr;
			RigidBodyType2D Type				= RigidBodyType2D::Static;
			RigidBodyDef2D::BodyFlags Flags		= RigidBodyDef2D::BodyFlags::None;
			RigidBody2D() = default;
			// Only default constructor - creation of that component depends on rigid body world, and other things,
			// so it is handled by Scene object.
		};

		// Stores lightweight representation of physics engine version,
		// Simplifies creation process.
		struct BoxCollider2D
		{
			// TODO: only first collider is registered (maybe implement composite collider as a list?)
			// Pointer to real (physics engine's) collider.
			Engine::BoxCollider2D* PhysicsCollider	= nullptr;
			PhysicsMaterial PhysicsMaterial{};
			glm::vec2 Offset						= glm::vec2{ 0.0f };
			glm::vec2 HalfSize						= glm::vec2{ 0.5f };
			bool IsSensor							= false;
			BoxCollider2D() = default;
			// Only default constructor - creation of that component depends on rigid body world, and other things,
			// so it is handled by Scene object.
		};

		struct SpriteRenderer
		{
			Texture* Texture					= nullptr;
			std::array<glm::vec2, 4> UV			= {	glm::vec2{0.0f, 0.0f}, glm::vec2{1.0f, 0.0f}, glm::vec2{1.0f, 1.0f}, glm::vec2{0.0f, 1.0f} };
			glm::vec4 Tint						= glm::vec4{1.0f};
			glm::vec2 Tiling					= glm::vec2{1.0f};
			SortingLayer::Layer SortingLayer	= DefaultSortingLayer.GetDefaultLayer();
			I16 OrderInLayer					= 0;
			bool FlipX = false; bool FlipY = false;
			SpriteRenderer(Engine::Texture* texture, const std::array<glm::vec2, 4> uv, const glm::vec4& tint, const glm::vec2& tiling,
			               SortingLayer::Layer layer, I16 orderInLayer) 
				: Texture(texture), UV(uv), Tint(tint), Tiling(tiling), SortingLayer(std::move(layer)), OrderInLayer(orderInLayer)
			{ }
			SpriteRenderer() = default;
		};
		
		struct PolygonRenderer
		{
			RegularPolygon* Polygon				= nullptr;
			Texture* Texture					= nullptr;
			glm::vec4 Tint						= glm::vec4{1.0f};
			glm::vec2 Tiling					= glm::vec2{1.0f};
			SortingLayer::Layer SortingLayer	= DefaultSortingLayer.GetDefaultLayer();
			I16 OrderInLayer					= 0;
			bool FlipX = false; bool FlipY = false;
			PolygonRenderer(RegularPolygon* polygon, Engine::Texture* texture, const glm::vec4& tint, const glm::vec2& tiling,
							SortingLayer::Layer layer, I16 orderInLayer)
				: Polygon(polygon), Texture(texture), Tint(tint), Tiling(tiling), SortingLayer(std::move(layer)), OrderInLayer(orderInLayer)
			{ }
			PolygonRenderer() = default;
		};

		struct FontRenderer
		{
			struct Rect
			{
				glm::vec2 Min = glm::vec2{0.0f}; glm::vec2 Max = glm::vec2{1.0f};
			};
			Font* Font							= nullptr;
			F32 FontSize						= 32;
			Rect FontRect{};
			glm::vec4 Tint						= glm::vec4{1.0f};
			SortingLayer::Layer SortingLayer	= DefaultSortingLayer.GetDefaultLayer();
			I16 OrderInLayer					= 0;
			FontRenderer(Engine::Font* font, F32 fontSize, const Rect& fontRect, const glm::vec4& tint,
						 SortingLayer::Layer layer, I16 orderInLayer)
				: Font(font), FontSize(fontSize), FontRect(fontRect), Tint(tint), SortingLayer(std::move(layer)), OrderInLayer(orderInLayer)
			{ }
			FontRenderer() = default;
		};

		// Static checks that components are pod.
		static_assert(std::is_trivially_copyable_v<Component::Transform2D>);
		static_assert(std::is_trivially_copyable_v<Component::RigidBody2D>);
		static_assert(std::is_trivially_copyable_v<Component::BoxCollider2D>);
		static_assert(std::is_trivially_copyable_v<Component::SpriteRenderer>);
		static_assert(std::is_trivially_copyable_v<Component::PolygonRenderer>);
		static_assert(std::is_trivially_copyable_v<Component::FontRenderer>);
		
		struct Mesh2D
		{
			RegularPolygon Shape;
			glm::vec4 Tint;
			glm::vec2 Tiling;
			std::vector<glm::vec2> UV;
			Texture* Texture;
			//TODO: Most of the parameters one day shall become the part of `Material`.
			Mesh2D(U32 angles, Engine::Texture* texture, const glm::vec4& tint) :
				Shape(angles), Tint(tint), Tiling(glm::vec2{1.0f}), UV(Shape.GetUVs()), Texture(texture)
			{ }
			Mesh2D(U32 angles) :
				Shape(angles), Tint(glm::vec4{ 1.0f }), Tiling(glm::vec2{1.0f}), UV(Shape.GetUVs()), Texture(nullptr)
			{ }
		};


		/************** App specific. *********************************/

		/************** MarioGame *************************************/
		struct MarioInput
		{
			bool CanJump = false;
			bool Jump = false;
			bool Left = false;
			bool Right = false;
			bool None = false;
		};
		/************** MarioGame *************************************/

		struct GemWarsTransform2D
		{
			glm::vec3 Position;
			glm::vec2 Scale;
			F32 Rotation;
			GemWarsTransform2D(const glm::vec3& pos, const glm::vec2& scale, F32 rotation) :
				Position(pos), Scale(scale), Rotation(rotation)
			{ }
			GemWarsTransform2D(const glm::vec2& pos, const glm::vec2& scale, F32 rotation) :
				Position(glm::vec3(pos, 0.0f)), Scale(scale), Rotation(rotation)
			{ }
		};

		struct GemWarsRigidBody2D
		{
			F32 CollisionRadius;
			F32 Speed;
			F32 RotationSpeed;
			glm::vec2 Velocity;
			GemWarsRigidBody2D(F32 colRadius, F32 speed, F32 rotationSpeed = 0.0f) :
				CollisionRadius(colRadius), Speed(speed), RotationSpeed(rotationSpeed), Velocity(glm::vec2(0.0f))
			{ }
		};

		struct GemWarsLifeSpan
		{
			I32 Remaining;
			I32 Total;
			GemWarsLifeSpan(I32 total) :
				Remaining(total), Total(total)
			{ }
		};

		struct GemWarsInput
		{
			bool Up, Down, Left, Right;
			bool Shoot, SpecialAbility;
			GemWarsInput() : 
				Up(false), Down(false), Left(false), Right(false), Shoot(false), SpecialAbility(false)
			{ }
		};

		struct GemWarsSpecialAbility
		{
			I32 RemainingCoolDown;
			I32 CoolDown;
			GemWarsSpecialAbility(I32 coolDown) :
				RemainingCoolDown(0), CoolDown(coolDown)
			{ }
		};

		struct GemWarsScore
		{
			U32 TotalScore;
			GemWarsScore(U32 score) : TotalScore(score) 
			{ }
		};
	}
	
}
