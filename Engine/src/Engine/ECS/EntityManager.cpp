#include "enginepch.h"

#include "Engine/ECS/EntityManager.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	EntityManager::EntityManager() = default;
	
	void EntityManager::Update()
	{
		// Delete all aliven't entities.
		for (const auto& entity : m_Entities)
		{
			if (!entity->IsActive)	m_IdToEntity.erase(static_cast<I32>(entity->Id));
		}
		std::erase_if(m_Entities, [](auto ent) { return !ent->IsActive; });
		for (auto& entityList : m_EntityMap | std::views::values)
		{
			std::erase_if(entityList, [](auto ent) { return !ent->IsActive; });
		}
		
		// Add all new entities.
		for (auto& e : m_ToAdd)
		{
			m_Entities.push_back(e);

			if (!m_EntityMap.contains(e->Tag)) m_EntityMap.insert({ e->Tag, {e} });
			else m_EntityMap[e->Tag].push_back(e);
		}

		m_ToAdd.clear();
	}
	
	Entity& EntityManager::AddEntity(const std::string& tag)
	{
		const auto entity = CreateRef<Entity>(tag, m_TotalEntities++);
		m_ToAdd.push_back(entity);
		m_IdToEntity[static_cast<I32>(entity->Id)] = entity.get();
		return *entity;
	}
	
	EntityVector& EntityManager::GetEntities()
	{
		return m_Entities;
	}
	
	EntityVector& EntityManager::GetEntities(const std::string& tag)
	{
		if (!m_EntityMap.contains(tag))
		{
			// Here we add empty vector for that tag to map, and return it.
			m_EntityMap.insert({ tag, {} });
		}
		return m_EntityMap[tag];
	}
}