#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Rendering/Buffer.h"
#include "Engine/Primitives/Shape.h"

#include <glm/glm.hpp>

#include "Polygon.h"

namespace Engine
{
	using namespace Types;
	class RegularPolygon : public Polygon
	{
	public:
		RegularPolygon() = default;
		RegularPolygon(U32 angles, bool genUV = true);
	private:
		U32 m_Angles;
	};
}
