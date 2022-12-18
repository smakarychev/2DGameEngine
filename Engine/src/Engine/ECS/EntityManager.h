#pragma once

#include "Engine/Common/SparseSetPaged.h"
#include "Engine/ECS/EntityId.h"

namespace Engine
{
	using EntityContainer = SparseSetPaged<U32, Entity, EntityIdDecomposer>;
	class EntityManager
	{
		friend class Registry;
	public:
		using EntityVector = std::vector<Entity>;
		EntityManager() = default;

		Entity AddEntity(const std::string& tag = "Default");

		void DeleteEntity(Entity entityId);

		bool IsAlive(Entity entityId);

		U32 GetNullEntityFlag() const { return m_EntitiesSparseSet.GetNullFlag(); }
		
	private:
		struct EntityIdTag
		{
			Entity Entity;
			std::string Tag;
		};
		EntityContainer m_EntitiesSparseSet;
		std::unordered_map<std::string, EntityContainer> m_EntitiesMap;
		EntityVector m_FreeEntities{};

		U32 m_TotalEntities = 0;
	};
}