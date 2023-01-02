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

#include "EntityId.h"
#include "Engine/Memory/Handle/Handle.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Contacts.h"

namespace Engine
{
    class CameraController;
}

namespace Engine
{
    class Registry;
}

#define CREATE_TAG_COMPONENT(tag)   struct tag{};                                                                                           \
                                    class tag##Serializer : public Engine::ComponentSerializer<tag>                                         \
                                    {                                                                                                       \
                                    public:                                                                                                 \
                                        COMPONENT_SERIALIZER_SIGNATURE(tag)                                                                 \
                                        tag##Serializer(Engine::Scene& scene)                                                               \
                                            : ComponentSerializer<tag>(GetStaticSignature(), scene) {}                                      \
                                        void SerializeComponent(const tag& component, YAML::Emitter& emitter) override                      \
                                        {                                                                                                   \
                                            emitter << YAML::Key << m_ComponentSignature << YAML::BeginMap << YAML::EndMap;                 \
                                        }                                                                                                   \
                                        void DeserializeComponent(Engine::Entity e, YAML::Node& node) override                              \
                                        {                                                                                                   \
                                            auto tagS = node[m_ComponentSignature];                                                         \
                                        auto& tagC = m_Registry.AddOrGet<tag>(e);                                                           \
                                        }                                                                                                   \
                                        bool SupportsCreationInEditor() override { return true; }                                           \
                                        void AddEmptyComponentTo(Engine::Entity e) override                                                 \
                                        {                                                                                                   \
                                            if (!m_Registry.Has<tag>(e))  auto& tagC = m_Registry.Add<tag>(e);                              \
                                        }                                                                                                   \
                                    };                                                                                                      \
                                    class tag##UIDesc : public Engine::ComponentUIDesc<tag>                                                 \
                                    {                                                                                                       \
                                    public:                                                                                                 \
                                        COMPONENT_UI_DESC_SIGNATURE(tag)                                                                    \
                                        tag##UIDesc(Engine::Scene& scene) : ComponentUIDesc(GetStaticSignature(), true, scene){}   \
                                        void OnUIDraw(Engine::Entity e, tag& component) override {}                                         \
                                    };                                                                                                      \



namespace Engine::Component
{
    struct LocalToParentTransform2D;
    using namespace Types;

    struct Transform
    {
        glm::vec3 Position;
        glm::quat Rotation;
        glm::vec3 Scale;

        Transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
    };

    // Placeholder for real camera component which is yet to be added.
    struct Camera
    {
        Ref<CameraController> CameraController{nullptr};
        // TODO: I'm not so sure it belongs here (but they are very tied together).
        Ref<FrameBuffer> CameraFrameBuffer{nullptr};
        bool IsPrimary{false};
    };

    struct Name
    {
        std::string EntityName;
    };
    
    struct LocalToWorldTransform2D
    {
        glm::vec2 Position{glm::vec2{0.0f}};
        glm::vec2 Scale{glm::vec2{1.0f}};
        Rotation Rotation{glm::vec2{1.0f, 0.0f}};

        LocalToWorldTransform2D();
        LocalToWorldTransform2D(const glm::vec2& pos, const glm::vec2& scale, const glm::vec2& rotation);
        LocalToWorldTransform2D(const glm::vec2& pos, const glm::vec2& scale, F32 rotation);
        LocalToWorldTransform2D(const LocalToParentTransform2D& transform);
        LocalToWorldTransform2D Concatenate(const LocalToWorldTransform2D& other);
        LocalToWorldTransform2D Inverse();

        // Very common functions, so it makes sense to just put it here.
        glm::vec2 Transform(const glm::vec2& point) const;
        glm::vec2 TransformDirection(const glm::vec2& dir) const;
        glm::vec2 InverseTransform(const glm::vec2& point) const;
        glm::vec2 InverseTransformDirection(const glm::vec2& dir) const;
    };

    struct LocalToParentTransform2D
    {
        glm::vec2 Position{glm::vec2{0.0f}};
        glm::vec2 Scale{glm::vec2{1.0f}};
        Rotation Rotation{glm::vec2{1.0f, 0.0f}};
        
        LocalToParentTransform2D();
        LocalToParentTransform2D(const LocalToWorldTransform2D& transform);
    };

    struct ChildRel
    {
        U32 ChildrenCount{0};
        Entity First{NULL_ENTITY};
    };

    struct ParentRel
    {
        U32 Depth{1};
        Entity Parent{NULL_ENTITY};
        Entity Next{NULL_ENTITY};
        Entity Prev{NULL_ENTITY};
    };

    using PhysicsMaterial = Physics::PhysicsMaterial;

    // Stores lightweight representation of physics engine version,
    // Simplifies creation process.
    struct RigidBody2D
    {
        using RBHandle = RefCountHandle<Physics::RigidBody2D>;
        // Pointer to real (physics engine's) body.
        RBHandle PhysicsBody{nullptr};
        Physics::RigidBodyType2D Type{Physics::RigidBodyType2D::Static};
        Physics::RigidBodyDef2D::BodyFlags Flags{Physics::RigidBodyDef2D::BodyFlags::RestrictRotation};
        
        // Only default constructor - creation of that component depends on rigid body world, and other things,
        // so it is handled by Scene object.
        RigidBody2D();
    };

    // Stores lightweight representation of physics engine version,
    // Simplifies creation process.
    struct BoxCollider2D
    {
        using ColHandle = RefCountHandle<Physics::BoxCollider2D>;
        // Pointer to real (physics engine's) collider.
        ColHandle PhysicsCollider{nullptr};
        PhysicsMaterial PhysicsMaterial{};
        glm::vec2 Offset{glm::vec2{0.0f}};
        glm::vec2 HalfSize{glm::vec2{0.5f}};
        bool IsSensor{false};
        
        // Only default constructor - creation of that component depends on rigid body world, and other things,
        // so it is handled by Scene object.
        BoxCollider2D();
    };

    struct SpriteRenderer
    {
        Texture* Texture{nullptr};
        std::array<glm::vec2, 4> UV {
            glm::vec2{0.0f, 0.0f}, glm::vec2{1.0f, 0.0f}, glm::vec2{1.0f, 1.0f}, glm::vec2{0.0f, 1.0f}
        };
        glm::vec4 Tint{glm::vec4{1.0f}};
        glm::vec2 Tiling{glm::vec2{1.0f}};
        SortingLayer::Layer SortingLayer{DefaultSortingLayer.GetDefaultLayer()};
        I16 OrderInLayer{0};
        bool FlipX{false};
        bool FlipY{false};

        SpriteRenderer(Engine::Texture* texture, const std::array<glm::vec2, 4> uv, const glm::vec4& tint,
                       const glm::vec2& tiling,
                       SortingLayer::Layer layer, I16 orderInLayer);
        SpriteRenderer();
    };

    struct PolygonRenderer
    {
        RegularPolygon* Polygon{nullptr};
        Texture* Texture{nullptr};
        glm::vec4 Tint{glm::vec4{1.0f}};
        glm::vec2 Tiling{glm::vec2{1.0f}};
        SortingLayer::Layer SortingLayer{DefaultSortingLayer.GetDefaultLayer()};
        I16 OrderInLayer{0};
        bool FlipX{false};
        bool FlipY{false};

        PolygonRenderer(RegularPolygon* polygon, Engine::Texture* texture, const glm::vec4& tint,
                        const glm::vec2& tiling,
                        SortingLayer::Layer layer, I16 orderInLayer);
        PolygonRenderer();
    };

    struct FontRenderer
    {
        struct Rect
        {
            glm::vec2 Min{glm::vec2{0.0f}};
            glm::vec2 Max{glm::vec2{1.0f}};
        };

        Font* Font{nullptr};
        F32 FontSize{32};
        Rect FontRect{};
        glm::vec4 Tint{glm::vec4{1.0f}};
        SortingLayer::Layer SortingLayer{DefaultSortingLayer.GetDefaultLayer()};
        I16 OrderInLayer{0};

        FontRenderer(Engine::Font* font, F32 fontSize, const Rect& fontRect, const glm::vec4& tint,
                     SortingLayer::Layer layer, I16 orderInLayer);
        FontRenderer();
    };

    struct Animation
    {
        // TODO: maybe store animation in it's own manager, like textures?
        Ref<Engine::SpriteAnimation> SpriteAnimation{nullptr};
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
        Entity Top{NULL_ENTITY};
        Entity Bottom{NULL_ENTITY};
        Entity Left{NULL_ENTITY};
        Entity Right{NULL_ENTITY};
    };

    struct CollisionCallback
    {
        struct CollisionData
        {
            Entity Primary{NULL_ENTITY};
            Entity Secondary{NULL_ENTITY};
            Physics::ContactListener::ContactState ContactState{Physics::ContactListener::ContactState::Begin};
        };
        using SensorCallback = void (*)(Registry* registry, const CollisionData& collisionData,
                                        [[maybe_unused]] const Physics::ContactInfo2D& contact);
        I32 SensorCallbackIndex{-1};
        U32 CollisionCount{0};
    };


    /************** MarioGame *************************************/

    struct GemWarsTransform2D
    {
        glm::vec3 Position{};
        glm::vec2 Scale{};
        F32 Rotation{};

        GemWarsTransform2D(const glm::vec3& pos, const glm::vec2& scale, F32 rotation);
        GemWarsTransform2D(const glm::vec2& pos, const glm::vec2& scale, F32 rotation);
        GemWarsTransform2D();
    };

    struct GemWarsRigidBody2D
    {
        F32 CollisionRadius{};
        F32 Speed{};
        F32 RotationSpeed{};
        glm::vec2 Velocity{};

        GemWarsRigidBody2D(F32 colRadius, F32 speed, F32 rotationSpeed = 0.0f);
        GemWarsRigidBody2D();
    };

    struct GemWarsLifeSpan
    {
        I32 Remaining{};
        I32 Total{};

        GemWarsLifeSpan(I32 total);
        GemWarsLifeSpan();
    };

    struct GemWarsInput
    {
        bool Up, Down, Left, Right;
        bool Shoot, SpecialAbility;

        GemWarsInput();
    };

    struct GemWarsSpecialAbility
    {
        I32 RemainingCoolDown{};
        I32 CoolDown{};

        GemWarsSpecialAbility(I32 coolDown);
        GemWarsSpecialAbility();
    };

    struct GemWarsScore
    {
        U32 TotalScore{};

        GemWarsScore(U32 score);
        GemWarsScore();
    };

    struct GemWarsMesh2D
    {
        RegularPolygon Shape{};
        glm::vec4 Tint{};
        glm::vec2 Tiling{};
        std::vector<glm::vec2> UV;
        Texture* Texture{};
        //TODO: Most of the parameters one day shall become a part of `Material`.
        GemWarsMesh2D(U32 angles, Engine::Texture* texture, const glm::vec4& tint);

        GemWarsMesh2D(U32 angles);

        GemWarsMesh2D();
    };

    struct GemWarsPlayerTag
    {};
    struct GemWarsEnemyTag
    {};
    struct GemWarsBulletTag
    {};
    struct GemWarsParticleTag
    {};
}
