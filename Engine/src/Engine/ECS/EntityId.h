#pragma once

#include "Engine/Core/Types.h"

namespace Engine
{
	using namespace Types;
	struct EntityId
	{
		static constexpr U32 GENERATION_BITS = 8;
		static constexpr U32 GENERATION_SHIFT = 32 - GENERATION_BITS;
		static constexpr U32 GENERATION_MASK = static_cast<U32>((1 << GENERATION_BITS) - 1) << GENERATION_SHIFT;
		static constexpr U32 INDEX_MASK = ~GENERATION_MASK;
		U32 Id;
		U32 GetGeneration() const { return (Id & GENERATION_MASK) >> GENERATION_SHIFT; }
		U32 GetIndex() const { return Id & INDEX_MASK; }

		constexpr EntityId(U32 index = 0, U32 generation = 0)
		{
			Id = ((generation << GENERATION_SHIFT) & GENERATION_MASK) | index;
		}

		operator U32() const { return Id; }

		bool operator==(const EntityId& other) const
		{
			return Id == other.Id;
		}

		bool operator!=(const EntityId& other) const
		{
			return Id != other.Id;
		}
	};

	using Entity = EntityId;

	constexpr Entity NULL_ENTITY = std::numeric_limits<U32>::max() & Entity::INDEX_MASK;
	
	
	struct EntityIdDecomposer
	{
		static std::pair<U32, U32> Decompose(Entity id) { return std::make_pair(id.GetGeneration(), id.GetIndex()); }
	};

	struct EntityIdComposer
	{
		static EntityId Compose(U32 generation, U32 index) { return EntityId(index, generation); }
	};
}

namespace std
{
	template <typename T> struct hash;
	template<>
	struct hash<Engine::EntityId>
	{
		Engine::U64 operator()(const Engine::EntityId& entity) const noexcept
		{
			return static_cast<Engine::U64>(entity.Id);
		}
	};
}
