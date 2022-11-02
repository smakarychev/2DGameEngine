#pragma once

#include "Engine/ECS/Entity.h"

namespace Engine
{
	using namespace Types;
	using EntityVector = std::vector<std::shared_ptr<Entity>>;
	using EntityMap = std::unordered_map<std::string, EntityVector>;
	using IdToEntityMap = std::unordered_map<I32, Entity*>;
	
	class EntityManager
	{
	public:
		EntityManager();

		// Shall be called at the beginning of each frame.
		void Update();

		Entity& AddEntity(const std::string& tag);

		EntityVector& GetEntities();
		EntityVector& GetEntities(const std::string& tag);
		Entity* GetEntityById(I32 id) { return m_IdToEntity[id]; }

	private:
		EntityVector m_Entities;
		EntityMap m_EntityMap;
		// Later this will go, as entity will become id.
		IdToEntityMap m_IdToEntity;
		
		EntityVector m_ToAdd;

		U64 m_TotalEntities = 0;

	};
}