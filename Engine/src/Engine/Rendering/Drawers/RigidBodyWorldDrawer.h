#pragma once

#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"

#include "Engine/Rendering/Renderer2D.h"

#include <glm/gtc/type_ptr.hpp>

namespace Engine
{
	class RigidBodyWorldDrawer
	{
    public:
		static void Draw(const Physics::RigidBodyWorld2D& world);
	};

	inline void RigidBodyWorldDrawer::Draw(const Physics::RigidBodyWorld2D& world)
	{
	    using namespace Physics;
        for (const RigidBodyListEntry2D* bodyEntry = world.GetBodyList(); bodyEntry != nullptr; bodyEntry = bodyEntry->Next)
        {
            RigidBody2D* body = bodyEntry->Body;
            glm::vec4 bodyColor { 1.0f };
            switch (body->GetType())
            {
            case RigidBodyType2D::Dynamic:
                bodyColor = glm::vec4{ 0.31f, 0.45f, 0.38f, 1.0f };
                break;
            case RigidBodyType2D::Kinematic:
                bodyColor = glm::vec4{ 0.31f, 0.38f, 0.45f, 1.0f };
                break;
            case RigidBodyType2D::Static:
                bodyColor = glm::vec4{ 0.45f, 0.31f, 0.38f, 1.0f };
                break;
            }
            Component::Transform2D bodyTransform;
            bodyTransform.Position = body->GetTransform().Transform(body->GetCenterOfMass());
            bodyTransform.Scale = glm::vec2{0.05f, 0.05f};
            Component::SpriteRenderer sp;
            sp.Tint = {0.5f,0.5f,0.5f, 1.0f};
            Renderer2D::DrawQuad(bodyTransform, sp);
            for (const ColliderListEntry2D* colliderEntry = body->GetColliderList(); colliderEntry != nullptr; colliderEntry = colliderEntry->Next)
            {
                Collider2D* collider = colliderEntry->Collider;
                AABB2D bounds = collider->GenerateBounds();
                // Column wise.
                glm::mat3 translateBody{ 1.0f };
                translateBody[2][0] = body->GetPosition().x; translateBody[2][1] = body->GetPosition().y;
                glm::mat3 rotateBody{ 1.0f };
                rotateBody[0][0] =  body->GetRotation().x; rotateBody[0][1] = body->GetRotation().y;
                rotateBody[1][0] = -body->GetRotation().y; rotateBody[1][1] = body->GetRotation().x;

                glm::mat3 translateCol{ 1.0f };
                translateCol[2][0] = bounds.Center.x; translateCol[2][1] = bounds.Center.y;
                glm::mat3 scaleCol{ 1.0f };
                scaleCol[0][0] = bounds.HalfSize.x * 2.0f; scaleCol[1][1] = bounds.HalfSize.y * 2.0f;

                glm::mat3 transformBody = translateBody * rotateBody;
                glm::mat3 transfromCol = translateCol * scaleCol;
                glm::mat3 transform = transformBody * transfromCol;
                Component::SpriteRenderer colliderSp;
                colliderSp.Tint = { collider->IsSensor() ? glm::vec4{ 1.0f, 0.76f, 0.05f, 1.0f } : bodyColor };
                Renderer2D::DrawQuad(transform, colliderSp, RendererAPI::PrimitiveType::Line);
            }
           
        }
	}
}