#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"
#include "Engine/Math/MathUtils.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;

	//? Find a better place for it?

	// Represents rotation + translation in 2d space.
	struct Transform2D
	{
		glm::vec2 Translation = glm::vec2{ 0.0f, 0.0f };
		glm::vec2 Rotation = glm::vec2{ 1.0f, 0.0f };
		glm::vec2 Transform(const glm::vec2& point) const
		{
			return glm::vec2 {
			point.x * Rotation.x - point.y * Rotation.y + Translation.x,
			point.x * Rotation.y + point.y * Rotation.x + Translation.y
			};
		}
		glm::vec2 TransformDirection(const glm::vec2& dir) const
		{
			return glm::vec2 {
			dir.x * Rotation.x - dir.y * Rotation.y,
			dir.x * Rotation.y + dir.y * Rotation.x
			};
		}
		glm::vec2 InverseTransform(const glm::vec2& point) const
		{
			glm::vec2 translated = point - Translation;
			return glm::vec2 {
				 translated.x * Rotation.x + translated.y * Rotation.y,
				-translated.x * Rotation.y + translated.y * Rotation.x
			};
		}
		glm::vec2 InverseTransformDirection(const glm::vec2& dir) const
		{
			return glm::vec2 {
			 dir.x * Rotation.x + dir.y * Rotation.y,
			-dir.x * Rotation.y + dir.y * Rotation.x
			};
		}
	};

	struct Rect
	{
		glm::vec2 Center;
		glm::vec2 HalfSize;
		Rect(const glm::vec2& center = glm::vec2{ 0.0f }, const glm::vec2& halfSize = glm::vec2{ 1.0f }) : Center(center), HalfSize(halfSize)
		{}

		bool Contains(const glm::vec2& point) const
		{
			return Math::Abs(point.x - Center.x) < HalfSize.x &&
				Math::Abs(point.y - Center.y) < HalfSize.y;
		}

		bool Contains(const Rect& rect) const
		{
			return Math::Abs(rect.Center.x - Center.x) < (HalfSize.x - rect.HalfSize.x) &&
				Math::Abs(rect.Center.y - Center.y) < (HalfSize.y - rect.HalfSize.y);
		}

		bool Overlaps(const Rect& rect) const
		{
			return Math::Abs(rect.Center.x - Center.x) < (HalfSize.x + rect.HalfSize.x) &&
				Math::Abs(rect.Center.y - Center.y) < (HalfSize.y + rect.HalfSize.y);
		}

	};
}