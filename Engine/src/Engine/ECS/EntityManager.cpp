#include "enginepch.h"

#include "Engine/ECS/EntityManager.h"

namespace Engine
{
	EntityManager::EntityManager()
	{
	}
	
	void EntityManager::Update()
	{
		// Delete all aliven't entities.
		for (auto it = m_Entities.begin(); it != m_Entities.end();)
		{
			auto& ent = *it;
			if (!ent->IsAlive)
			{
				m_EntityMap[ent->Tag].erase(std::find(m_EntityMap[ent->Tag].begin(), m_EntityMap[ent->Tag].end(), ent));
				it = m_Entities.erase(it);
			}
			else
			{
				it++;
			}
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
		auto entity = std::make_shared<Entity>(tag, m_TotalEntites++);
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
			ENGINE_ERROR("EntityManager: unknown tag {}", tag);
			// Here we add empty vector for that tag to map, and return it.
			m_EntityMap.insert({ tag, {} });
		}
		return m_EntityMap[tag];
	}
}