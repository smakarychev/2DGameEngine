#pragma once

#include "Engine/ECS/EntityId.h"

namespace Engine
{
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
		SparseSet<U32, Entity, EntityIdDecomposer> m_EntitiesSparseSet;
		std::unordered_map<std::string, SparseSet<U32, Entity, EntityIdDecomposer>> m_EntitiesMap;
		EntityVector m_FreeEntities{};

		U32 m_TotalEntities = 0;
	};
}