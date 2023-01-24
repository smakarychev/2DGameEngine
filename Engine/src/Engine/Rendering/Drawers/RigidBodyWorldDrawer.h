#pragma once

#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"

#include "Engine/Rendering/Renderer2D.h"

#include <glm/gtc/type_ptr.hpp>

#include "Engine/Physics/NewRBE/RigidBodyWorld.h"

namespace Engine
{
	class RigidBodyWorldDrawer
	{
    public:
		static void Draw(const Physics::RigidBodyWorld2D& world);
		//TODO: TEMP
		static void Draw(const WIP::Physics::RigidBodyWorld2D& world);
	};

	inline void RigidBodyWorldDrawer::Draw(const Physics::RigidBodyWorld2D& world)
	{
	    using namespace Physics;

        const glm::vec4 dynamicBodyColliderColor = glm::vec4{ 0.31f, 0.45f, 0.38f, 1.0f };
        const glm::vec4 kinematicBodyColliderColor = glm::vec4{ 0.31f, 0.38f, 0.45f, 1.0f };
        const glm::vec4 staticBodyColliderColor = glm::vec4{ 0.45f, 0.31f, 0.38f, 1.0f };
	    const glm::vec4 bodilessColliderColor = glm::vec4{ 0.45f, 0.45f, 0.31f, 1.0f };

		for (const RigidBodyListEntry2D* bodyEntry = world.GetBodyList(); bodyEntry != nullptr; bodyEntry = bodyEntry->Next)
		{
			RigidBody2D* body = bodyEntry->Body;
			Component::LocalToWorldTransform2D bodyTransform;
			bodyTransform.Position = body->GetTransform().Transform(body->GetCenterOfMass());
			bodyTransform.Scale = glm::vec2{0.05f, 0.05f};
			Component::SpriteRenderer sp;
			sp.Tint = {0.5f,0.5f,0.5f, 1.0f};
			Renderer2D::DrawQuad(bodyTransform, sp);
		}

		for (const ColliderListEntry2D* colliderEntry = world.GetColliderList(); colliderEntry != nullptr; colliderEntry = colliderEntry->Next)
		{
			BoxCollider2D* collider = static_cast<BoxCollider2D*>(colliderEntry->Collider);
			AABB2D colliderBounds = collider->GenerateBounds(Component::LocalToWorldTransform2D{});
			Component::LocalToWorldTransform2D colliderTransform = collider->GetTransform();
			colliderTransform.Position = colliderTransform.Transform(collider->Center);
			colliderTransform.Scale = collider->HalfSize * 2.0f;
			Component::SpriteRenderer sp;
			RigidBody2D* body = collider->GetAttachedRigidBody();
			if (body)
			{
				switch (body->GetType())
				{
				case RigidBodyType2D::Dynamic: sp.Tint = dynamicBodyColliderColor; break;
				case RigidBodyType2D::Kinematic: sp.Tint = kinematicBodyColliderColor; break;
				case RigidBodyType2D::Static: sp.Tint = staticBodyColliderColor; break;
				}
			}
			else
			{
				sp.Tint = bodilessColliderColor;
			}
			Renderer2D::DrawQuad(colliderTransform, sp, RendererAPI::PrimitiveType::Line);
		}
	}

	inline void RigidBodyWorldDrawer::Draw(const WIP::Physics::RigidBodyWorld2D& world)
	{
        const glm::vec4 dynamicBodyColliderColor = glm::vec4{ 0.31f, 0.45f, 0.38f, 1.0f };
        const glm::vec4 kinematicBodyColliderColor = glm::vec4{ 0.31f, 0.38f, 0.45f, 1.0f };
        const glm::vec4 staticBodyColliderColor = glm::vec4{ 0.45f, 0.31f, 0.38f, 1.0f };

		for (auto& bodyEntry : world.GetBodyList())
		{
			WIP::Physics::RigidBody2D* body = bodyEntry.GetRigidBody();
			Component::LocalToWorldTransform2D bodyCoMTransform;
			bodyCoMTransform.Position = body->GetTransform().Transform(body->GetCenterOfMass());
			bodyCoMTransform.Scale = glm::vec2{0.05f, 0.05f};
			Component::LocalToWorldTransform2D bodyRealTransform;
			bodyRealTransform.Position = body->GetPosition();
			bodyRealTransform.Rotation = body->GetRotation();
			Component::SpriteRenderer sp;
			sp.Tint = {0.5f,0.5f,0.5f, 1.0f};
			Renderer2D::DrawQuad(bodyCoMTransform, sp);

			for (auto& colliderEntry : body->GetColliderList())
			{
				WIP::Physics::PolygonCollider2D* collider = static_cast<WIP::Physics::PolygonCollider2D*>(colliderEntry.GetCollider());
				WIP::Physics::AABB2D colliderBounds = collider->GenerateBounds(WIP::Physics::Transform2D{});
				Component::LocalToWorldTransform2D colliderTransform{};

				colliderTransform = colliderTransform.Concatenate(bodyRealTransform);
				Component::PolygonRenderer pr;
				Polygon polygon(collider->GetVertices());
				pr.Polygon = &polygon;
				switch (body->GetType())
				{
				case WIP::Physics::RigidBodyType2D::Dynamic: pr.Tint = dynamicBodyColliderColor; break;
				case WIP::Physics::RigidBodyType2D::Kinematic: pr.Tint = kinematicBodyColliderColor; break;
				case WIP::Physics::RigidBodyType2D::Static: pr.Tint = staticBodyColliderColor; break;
				}
	
				Renderer2D::DrawPolygon(colliderTransform, pr, RendererAPI::PrimitiveType::Line);
				for (auto& n : collider->GetNormals())
				{
					//Renderer2D::DrawLine(colliderTransform.Position, colliderTransform.Position + colliderTransform.TransformDirection(n) * 0.1f);
				}
			}
		}

	}
}
