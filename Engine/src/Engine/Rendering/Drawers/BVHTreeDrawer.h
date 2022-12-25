#pragma once

#include "Engine/Math/MathUtils.h"

#include "Engine/Physics/RigidBodyEngine/Collision/BroadPhase.h"
#include "Engine/Rendering/Renderer2D.h"

namespace Engine
{
	class BVHTreeDrawer
	{
	public:
		static void Draw(const Physics::BVHTree2D<Physics::AABB2D>& tree);
	};

	inline void BVHTreeDrawer::Draw(const Physics::BVHTree2D<Physics::AABB2D>& tree)
	{
		for (const auto& node : tree.m_Nodes)
		{
			if (node.IsLeaf())
			{
				Component::LocalToWorldTransform2D transform;
				transform.Position = node.Bounds.Center;
				transform.Scale = node.Bounds.HalfSize * 2.0f;
				Component::SpriteRenderer sp;
				sp.Tint = {0.0f, 0.0f, 0.8f, 0.8f};
				Renderer2D::DrawQuad(transform, sp, RendererAPI::PrimitiveType::Line);
			}
		}
		for (const auto& node : tree.m_Nodes)
		{
			if (node.Height > 0)
			{
				Component::LocalToWorldTransform2D transform;
				transform.Position = node.Bounds.Center;
				transform.Scale = node.Bounds.HalfSize * 2.0f * 1.01f;
				Component::SpriteRenderer sp;
				sp.Tint = {Math::Clamp(0.1f * node.Height, 0.0f, 1.0f), Math::Clamp(0.8f - 0.1f * node.Height, 0.0f, 1.0f), 0.0f, 0.5f};
				Renderer2D::DrawQuad(transform, sp, RendererAPI::PrimitiveType::Line);
			}
		}
	}
}


