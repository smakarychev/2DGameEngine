#pragma once

#include "Engine/Physics/RigidBodyEngine/Collision/BroadPhase.h"
#include "Engine/Rendering/Renderer2D.h"

namespace Engine
{
	template <>
	class BVHTreeDrawer<BoxCollider2D>
	{
	public:
		static void Draw(const BVHTree<BoxCollider2D>& tree);
	};

	inline void BVHTreeDrawer<BoxCollider2D>::Draw(const BVHTree<BoxCollider2D>& tree)
	{
		for (const auto& node : tree.m_Nodes)
		{
			if (node.IsLeaf())
			{
				Renderer2D::DrawQuad({ 
					.Position{node.Bounds.Center},
					.Scale{node.Bounds.HalfSize * 2.0f},
					.Color{0.0f, 0.0f, 0.8f, 0.5f} }
				);
			}
			else if (node.Height != -1)
			{
				Renderer2D::DrawQuad({
					.Position{node.Bounds.Center},
					.Scale{node.Bounds.HalfSize * 2.0f},
					.Color{0.0f, 0.8f, 0.0f, 0.2f} }
				);
			}
		}
	}

}


