#pragma once

#include "Engine/Common/SparseSet.h"
#include "Engine/Physics/RigidBodyEngine/RigidBody.h"
#include "Engine/Primitives/2D/RegularPolygon.h"
#include "Engine/Rendering/Animation.h"
#include "Engine/Rendering/Font.h"
#include "Engine/Rendering/SortingLayer.h"
#include "Engine/Rendering/Texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <tuple>

#include "EntityId.h"
#include "Engine/Memory/Handle/Handle.h"

namespace Engine::Component
{
    using namespace Types;

    struct Transform
    {
        glm::vec3 Position;
        glm::quat Rotation;
        glm::vec3 Scale;

        Transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) :
            Position(pos), Rotation(rot), Scale(scale)
        {
        }
    };

    struct Tag
    {
        std::string TagName = "Default";

        Tag(const std::string tag) : TagName(tag)
        {
        }

        Tag() = default;
    };

    struct Transform2D
    {
        struct Rotation
        {
            glm::vec2 RotationVec;

            Rotation(const glm::vec2& rotation) : RotationVec(rotation)
            {
            }

            Rotation(F32 angleRad) : RotationVec(glm::cos(angleRad), glm::sin(angleRad))
            {
            }

            operator const glm::vec2&() const { return RotationVec; }
            operator glm::vec2&() { return RotationVec; }
            F32& operator[](I32 index) { return RotationVec[index]; }
            const F32& operator[](I32 index) const { return RotationVec[index]; }
        };

        glm::vec2 Position = glm::vec2{0.0f};
        glm::vec2 Scale = glm::vec2{1.0f};
        Rotation Rotation = glm::vec2{1.0f, 0.0f};

        Transform2D(const glm::vec2& pos, const glm::vec2& scale, const glm::vec2& rotation) :
            Position(pos), Scale(scale), Rotation(rotation)
        {
        }

        Transform2D(const glm::vec2& pos, const glm::vec2& scale, F32 rotation) :
            Position(pos), Scale(scale), Rotation(rotation)
        {
        }

        Transform2D() = default;

        // Very common functions, so it makes sense to just put it here.
        glm::vec2 Transform(const glm::vec2& point) const
        {
            return glm::vec2 {
                point.x * Rotation.RotationVec.x - point.y * Rotation.RotationVec.y + Position.x,
                point.x * Rotation.RotationVec.y + point.y * Rotation.RotationVec.x + Position.y
                };
        }
        glm::vec2 TransformDirection(const glm::vec2& dir) const
        {
            return glm::vec2 {
                dir.x * Rotation.RotationVec.x - dir.y * Rotation.RotationVec.y,
                dir.x * Rotation.RotationVec.y + dir.y * Rotation.RotationVec.x
                };
        }
        glm::vec2 InverseTransform(const glm::vec2& point) const
        {
            glm::vec2 translated = point - Position;
            return glm::vec2 {
                translated.x * Rotation.RotationVec.x + translated.y * Rotation.RotationVec.y,
               -translated.x * Rotation.RotationVec.y + translated.y * Rotation.RotationVec.x
           };
        }
        glm::vec2 InverseTransformDirection(const glm::vec2& dir) const
        {
            return glm::vec2 {
                dir.x * Rotation.RotationVec.x + dir.y * Rotation.RotationVec.y,
               -dir.x * Rotation.RotationVec.y + dir.y * Rotation.RotationVec.x
               };
        }
        
    };

    using PhysicsMaterial = Physics::PhysicsMaterial;

    // Stores lightweight representation of physics engine version,
    // Simplifies creation process.
    struct RigidBody2D
    {
        using RBHandle = RefCountHandle<Physics::RigidBody2D>;
        // Pointer to real (physics engine's) body.
        RBHandle PhysicsBody = nullptr;
        Physics::RigidBodyType2D Type = Physics::RigidBodyType2D::Static;
        Physics::RigidBodyDef2D::BodyFlags Flags = Physics::RigidBodyDef2D::BodyFlags::None;
        RigidBody2D() = default;
        // Only default constructor - creation of that component depends on rigid body world, and other things,
        // so it is handled by Scene object.
    };

    // Stores lightweight representation of physics engine version,
    // Simplifies creation process.
    struct BoxCollider2D
    {
        using ColHandle = RefCountHandle<Physics::BoxCollider2D>;
        // Pointer to real (physics engine's) collider.
        ColHandle PhysicsCollider = nullptr;
        PhysicsMaterial PhysicsMaterial{};
        glm::vec2 Offset = glm::vec2{0.0f};
        glm::vec2 HalfSize = glm::vec2{0.5f};
        bool IsSensor = false;
        BoxCollider2D() = default;
        // Only default constructor - creation of that component depends on rigid body world, and other things,
        // so it is handled by Scene object.
    };

    struct SpriteRenderer
    {
        Texture* Texture = nullptr;
        std::array<glm::vec2, 4> UV = {
            glm::vec2{0.0f, 0.0f}, glm::vec2{1.0f, 0.0f}, glm::vec2{1.0f, 1.0f}, glm::vec2{0.0f, 1.0f}
        };
        glm::vec4 Tint = glm::vec4{1.0f};
        glm::vec2 Tiling = glm::vec2{1.0f};
        SortingLayer::Layer SortingLayer = DefaultSortingLayer.GetDefaultLayer();
        I16 OrderInLayer = 0;
        bool FlipX = false;
        bool FlipY = false;

        SpriteRenderer(Engine::Texture* texture, const std::array<glm::vec2, 4> uv, const glm::vec4& tint,
                       const glm::vec2& tiling,
                       SortingLayer::Layer layer, I16 orderInLayer)
            : Texture(texture), UV(uv), Tint(tint), Tiling(tiling), SortingLayer(std::move(layer)),
              OrderInLayer(orderInLayer)
        {
        }

        SpriteRenderer() = default;
    };

    struct PolygonRenderer
    {
        RegularPolygon* Polygon = nullptr;
        Texture* Texture = nullptr;
        glm::vec4 Tint = glm::vec4{1.0f};
        glm::vec2 Tiling = glm::vec2{1.0f};
        SortingLayer::Layer SortingLayer = DefaultSortingLayer.GetDefaultLayer();
        I16 OrderInLayer = 0;
        bool FlipX = false;
        bool FlipY = false;

        PolygonRenderer(RegularPolygon* polygon, Engine::Texture* texture, const glm::vec4& tint,
                        const glm::vec2& tiling,
                        SortingLayer::Layer layer, I16 orderInLayer)
            : Polygon(polygon), Texture(texture), Tint(tint), Tiling(tiling), SortingLayer(std::move(layer)),
              OrderInLayer(orderInLayer)
        {
        }

        PolygonRenderer() = default;
    };

    struct FontRenderer
    {
        struct Rect
        {
            glm::vec2 Min = glm::vec2{0.0f};
            glm::vec2 Max = glm::vec2{1.0f};
        };

        Font* Font = nullptr;
        F32 FontSize = 32;
        Rect FontRect{};
        glm::vec4 Tint = glm::vec4{1.0f};
        SortingLayer::Layer SortingLayer = DefaultSortingLayer.GetDefaultLayer();
        I16 OrderInLayer = 0;

        FontRenderer(Engine::Font* font, F32 fontSize, const Rect& fontRect, const glm::vec4& tint,
                     SortingLayer::Layer layer, I16 orderInLayer)
            : Font(font), FontSize(fontSize), FontRect(fontRect), Tint(tint), SortingLayer(std::move(layer)),
              OrderInLayer(orderInLayer)
        {
        }

        FontRenderer() = default;
    };

    struct Animation
    {
        Engine::SpriteAnimation* SpriteAnimation = nullptr;
    };

    struct GemWarsMesh2D
    {
        RegularPolygon Shape;
        glm::vec4 Tint{};
        glm::vec2 Tiling{};
        std::vector<glm::vec2> UV;
        Texture* Texture{};
        //TODO: Most of the parameters one day shall become a part of `Material`.
        GemWarsMesh2D(U32 angles, Engine::Texture* texture, const glm::vec4& tint) :
            Shape(angles), Tint(tint), Tiling(glm::vec2{1.0f}), UV(Shape.GetUVs()), Texture(texture)
        {
        }

        GemWarsMesh2D(U32 angles) :
            Shape(angles), Tint(glm::vec4{1.0f}), Tiling(glm::vec2{1.0f}), UV(Shape.GetUVs()), Texture(nullptr)
        {
        }

        GemWarsMesh2D() = default;
    };


    /************** App specific. *********************************/

    /************** MarioGame *************************************/
    // NOTE: 'Mario' here is not indicating that this component belongs to the Mario himself,
    // it is merely showing that it is a part of the Mario game.
    struct MarioInput
    {
        bool CanJump{false};
        bool Jump{false};
        bool Left{false};
        bool Right{false};
        bool None{false};
    };

    struct MarioState
    {
        bool IsInMidAir{false};
        bool IsInFreeFall{false};
        bool IsMovingLeft{false};
        bool IsMovingRight{false};
    };

    struct Sensors
    {
        Entity Bottom{NULL_ENTITY};
        Entity Left{NULL_ENTITY};
        Entity Right{NULL_ENTITY};
    };

    struct ChildRel
    {
        U32 ChildrenCount{0};
        Entity First{NULL_ENTITY};
    };

    struct ParentRel
    {
        Entity Parent{NULL_ENTITY};
        Entity Next{NULL_ENTITY};
        Entity Prev{NULL_ENTITY};
    };
    
    /************** MarioGame *************************************/

    struct GemWarsTransform2D
    {
        glm::vec3 Position;
        glm::vec2 Scale;
        F32 Rotation;

        GemWarsTransform2D(const glm::vec3& pos, const glm::vec2& scale, F32 rotation) :
            Position(pos), Scale(scale), Rotation(rotation)
        {
        }

        GemWarsTransform2D(const glm::vec2& pos, const glm::vec2& scale, F32 rotation) :
            Position(glm::vec3(pos, 0.0f)), Scale(scale), Rotation(rotation)
        {
        }

        GemWarsTransform2D() = default;
    };

    struct GemWarsRigidBody2D
    {
        F32 CollisionRadius;
        F32 Speed;
        F32 RotationSpeed;
        glm::vec2 Velocity;

        GemWarsRigidBody2D(F32 colRadius, F32 speed, F32 rotationSpeed = 0.0f) :
            CollisionRadius(colRadius), Speed(speed), RotationSpeed(rotationSpeed), Velocity(glm::vec2(0.0f))
        {
        }

        GemWarsRigidBody2D() = default;
    };

    struct GemWarsLifeSpan
    {
        I32 Remaining;
        I32 Total;

        GemWarsLifeSpan(I32 total) :
            Remaining(total), Total(total)
        {
        }

        GemWarsLifeSpan() = default;
    };

    struct GemWarsInput
    {
        bool Up, Down, Left, Right;
        bool Shoot, SpecialAbility;

        GemWarsInput() :
            Up(false), Down(false), Left(false), Right(false), Shoot(false), SpecialAbility(false)
        {
        }
    };

    struct GemWarsSpecialAbility
    {
        I32 RemainingCoolDown;
        I32 CoolDown;

        GemWarsSpecialAbility(I32 coolDown) :
            RemainingCoolDown(0), CoolDown(coolDown)
        {
        }

        GemWarsSpecialAbility() = default;
    };

    struct GemWarsScore
    {
        U32 TotalScore;

        GemWarsScore(U32 score) : TotalScore(score)
        {
        }

        GemWarsScore() = default;
    };
}
