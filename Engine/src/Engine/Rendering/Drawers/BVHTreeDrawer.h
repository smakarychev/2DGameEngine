#pragma once

#include "Engine/Math/MathUtils.h"

#include "Engine/Physics/RigidBodyEngine/Collision/BroadPhase.h"
#include "Engine/Rendering/Renderer2D.h"

namespace Engine
{
	template <>
	class BVHTreeDrawer<BoxCollider2D>
	{
	public:
		static void Draw(const BVHTree2D<BoxCollider2D>& tree);
	};

	inline void BVHTreeDrawer<BoxCollider2D>::Draw(const BVHTree2D<BoxCollider2D>& tree)
	{
		for (const auto& node : tree.m_Nodes)
		{
			if (node.IsLeaf())
			{
				Renderer2D::DrawQuad({ 
					.Position{node.Bounds.Center},
					.Scale{node.Bounds.HalfSize * 2.0f},
					.Color{0.0f, 0.0f, 0.8f, 0.8f},
					.Type{RendererAPI::PrimitiveType::Line} }
				);
			}
		}
		for (const auto& node : tree.m_Nodes)
		{
			if (node.Height != -1 && node.IsLeaf() == false)
			{
				Renderer2D::DrawQuad({
					.Position{node.Bounds.Center},
					.Scale{node.Bounds.HalfSize * 2.0f * (1.0f + 0.01f * node.Height)},
					.Color{Math::Clamp(0.2f * node.Height, 0.0f, 1.0f), Math::Clamp(0.8f - 0.2f * node.Height, 0.0f, 1.0f), 0.0f, 0.5f},
					.Type{RendererAPI::PrimitiveType::Line} }
				);
			}
		}
	}

}


