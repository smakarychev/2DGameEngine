#include "enginepch.h"
#include "Components.h"

#include "Engine/Math/LinearAlgebra.h"

namespace Engine::Component
{
    Transform::Transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
        : Position(pos), Rotation(rot), Scale(scale)
    {
    }

    LocalToWorldTransform2D::LocalToWorldTransform2D(const glm::vec2& pos, const glm::vec2& scale,
                                                                const glm::vec2& rotation)
        : Position(pos), Scale(scale), Rotation(rotation)
    {
    }

    LocalToWorldTransform2D::LocalToWorldTransform2D(const glm::vec2& pos, const glm::vec2& scale,
                                                                F32 rotation)
        : Position(pos), Scale(scale), Rotation(rotation)
    {
    }

    LocalToWorldTransform2D::LocalToWorldTransform2D(const LocalToParentTransform2D& transform)
        : Position(transform.Position), Scale(transform.Scale), Rotation(transform.Rotation)
    {
    }

    LocalToWorldTransform2D LocalToWorldTransform2D::Concatenate(
        const LocalToWorldTransform2D& other)
    {
        LocalToWorldTransform2D result;
        result.Rotation = Math::CombineRotation(this->Rotation, other.Rotation);
        result.Scale = this->Scale * other.Scale;
        result.Position = Math::Rotate(other.Scale * this->Position, other.Rotation) + other.Position;
        return result;
    }

    LocalToWorldTransform2D LocalToWorldTransform2D::Inverse()
    {
        LocalToWorldTransform2D result;
        result.Rotation = glm::vec2{ Rotation.RotationVec.x, -Rotation.RotationVec.y };
        result.Scale = 1.0f / Scale;
        result.Position = -result.Scale * Math::Rotate(this->Position, result.Rotation);
        return result;
    }

    LocalToWorldTransform2D::LocalToWorldTransform2D() = default;

    glm::vec2 LocalToWorldTransform2D::Transform(const glm::vec2& point) const
    {
        return Math::Rotate(point, Rotation) + Position;
    }

    glm::vec2 LocalToWorldTransform2D::TransformDirection(const glm::vec2& dir) const
    {
        return Math::Rotate(dir, Rotation);
    }

    glm::vec2 LocalToWorldTransform2D::InverseTransform(const glm::vec2& point) const
    {
        glm::vec2 translated = point - Position;
        return glm::vec2{
            translated.x * Rotation.RotationVec.x + translated.y * Rotation.RotationVec.y,
            -translated.x * Rotation.RotationVec.y + translated.y * Rotation.RotationVec.x
        };
    }

    glm::vec2 LocalToWorldTransform2D::InverseTransformDirection(const glm::vec2& dir) const
    {
        return glm::vec2{
            dir.x * Rotation.RotationVec.x + dir.y * Rotation.RotationVec.y,
            -dir.x * Rotation.RotationVec.y + dir.y * Rotation.RotationVec.x
        };
    }

    LocalToParentTransform2D::LocalToParentTransform2D() = default;

    LocalToParentTransform2D::LocalToParentTransform2D(const LocalToWorldTransform2D& transform)
        : Position(transform.Position), Scale(transform.Scale), Rotation(transform.Rotation)
    {
    }

    RigidBody2D::RigidBody2D() = default;
    BoxCollider2D::BoxCollider2D() = default;
    
    SpriteRenderer::SpriteRenderer(Engine::Texture* texture, const std::array<glm::vec2, 4> uv,
                                              const glm::vec4& tint, const glm::vec2& tiling, SortingLayer::Layer layer,
                                              I16 orderInLayer)
        : Texture(texture), UV(uv), Tint(tint), Tiling(tiling), SortingLayer(std::move(layer)),
          OrderInLayer(orderInLayer)
    {
    }

    SpriteRenderer::SpriteRenderer() = default;

    PolygonRenderer::PolygonRenderer(RegularPolygon* polygon, Engine::Texture* texture,
                                                const glm::vec4& tint, const glm::vec2& tiling,
                                                SortingLayer::Layer layer, I16 orderInLayer)
        : Polygon(polygon), Texture(texture), Tint(tint), Tiling(tiling), SortingLayer(std::move(layer)),
          OrderInLayer(orderInLayer)
    {
    }

    PolygonRenderer::PolygonRenderer() = default;

    FontRenderer::FontRenderer(Engine::Font* font, F32 fontSize, const Rect& fontRect, const glm::vec4& tint,
                                          SortingLayer::Layer layer, I16 orderInLayer)
        : Font(font), FontSize(fontSize), FontRect(fontRect), Tint(tint), SortingLayer(std::move(layer)),
          OrderInLayer(orderInLayer)
    {
    }

    FontRenderer::FontRenderer() = default;

    GemWarsTransform2D::GemWarsTransform2D(const glm::vec3& pos, const glm::vec2& scale, F32 rotation)
        : Position(pos), Scale(scale), Rotation(rotation)
    {
    }

    GemWarsTransform2D::GemWarsTransform2D(const glm::vec2& pos, const glm::vec2& scale, F32 rotation)
        : Position(glm::vec3(pos, 0.0f)), Scale(scale), Rotation(rotation)
    {
    }

    GemWarsTransform2D::GemWarsTransform2D() = default;

    GemWarsRigidBody2D::GemWarsRigidBody2D(F32 colRadius, F32 speed, F32 rotationSpeed)
        : CollisionRadius(colRadius), Speed(speed), RotationSpeed(rotationSpeed), Velocity(glm::vec2(0.0f))
    {
    }

    GemWarsRigidBody2D::GemWarsRigidBody2D() = default;

    GemWarsLifeSpan::GemWarsLifeSpan(I32 total)
        : Remaining(total), Total(total)
    {
    }

    GemWarsLifeSpan::GemWarsLifeSpan() = default;

    GemWarsInput::GemWarsInput()
        : Up(false), Down(false), Left(false), Right(false), Shoot(false), SpecialAbility(false)
    {
    }

    GemWarsSpecialAbility::GemWarsSpecialAbility(I32 coolDown)
        : RemainingCoolDown(0), CoolDown(coolDown)
    {
    }

    GemWarsSpecialAbility::GemWarsSpecialAbility() = default;

    GemWarsScore::GemWarsScore(U32 score)
        : TotalScore(score)
    {
    }

    GemWarsScore::GemWarsScore() = default;

    GemWarsMesh2D::GemWarsMesh2D(U32 angles, Engine::Texture* texture, const glm::vec4& tint)
        : Shape(angles), Tint(tint), Tiling(glm::vec2{1.0f}), UV(Shape.GetUVs()), Texture(texture)
    {
    }

    GemWarsMesh2D::GemWarsMesh2D(U32 angles)
        : Shape(angles), Tint(glm::vec4{1.0f}), Tiling(glm::vec2{1.0f}), UV(Shape.GetUVs()), Texture(nullptr)
    {
    }

    GemWarsMesh2D::GemWarsMesh2D() = default;
}
