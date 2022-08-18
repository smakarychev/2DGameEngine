#include "enginepch.h"

#include "Engine/ECS/EntityManager.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	EntityManager::EntityManager()
	{
	}
	
	void EntityManager::Update()
	{
		// Delete all aliven't entities.
		std::erase_if(m_Entities, [](auto ent) { return !ent->IsActive; });
		for (auto&& [tag, entityList] : m_EntityMap)
		{
			std::erase_if(entityList, [](auto ent) { return !ent->IsActive; });
		}

		// Add all new entities.
		for (auto& e : m_ToAdd)
		{
			m_Entities.push_back(e);

			if (m_EntityMap.find(e->Tag) == m_EntityMap.end()) m_EntityMap.insert({ e->Tag, {e} });
			else m_EntityMap[e->Tag].push_back(e);
		}

		m_ToAdd.clear();
	}
	
	Entity& EntityManager::AddEntity(const std::string& tag)
	{
		auto entity = std::shared_ptr<Entity>(New<Entity>(tag, m_TotalEntites++), Delete<Entity>);
		m_ToAdd.push_back(entity);
		return *entity;
	}
	
	EntityVector& EntityManager::GetEntities()
	{
		return m_Entities;
	}
	
	EntityVector& EntityManager::GetEntities(const std::string& tag)
	{
		if (m_EntityMap.find(tag) == m_EntityMap.end())
		{
			// Here we add empty vector for that tag to map, and return it.
			m_EntityMap.insert({ tag, {} });
		}
		return m_EntityMap[tag];
	}
}