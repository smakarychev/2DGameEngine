#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Rendering/Buffer.h"
#include "Engine/Primitives/Shape.h"

#include <glm/glm.hpp>
namespace Engine
{
	using namespace Types;
	class RegularPolygon
	{
	public:
		RegularPolygon() = default;
		RegularPolygon(U32 angles, bool genUV = true);

		const std::vector<glm::vec2>& GetVertices() const { return m_Vertices; }
		const std::vector<glm::vec2>& GetUVs() const { return m_UV; }
		const std::vector<U32>& GetIndices() const { return m_Indices; }
		const U32 GetNumberOfTriangles() const { return m_Angles - 2; }
		const U32 GetNumberOfVertices() const { return m_Angles; }
		UShapePrimitive GetPrimitiveType() const { return m_PrimitiveType; }

		void GenerateUVs(const std::vector<glm::vec2> uv);
	private:
		// TODO: custom containers?
		std::vector<glm::vec2> m_Vertices;
		std::vector<glm::vec2> m_UV;
		std::vector<U32> m_Indices;
		U32 m_Angles;
		UShapePrimitive m_PrimitiveType = UShapePrimitive::Triangles;
	};
}