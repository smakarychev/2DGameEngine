#pragma once

#include "Engine/ECS/Entity.h"

namespace Engine
{
	using EntityVector = std::vector<std::shared_ptr<Entity>>;
	using EntityMap = std::unordered_map<std::string, EntityVector>;
	
	class EntityManager
	{
	public:
		EntityManager();

		// Shall be called at the beginning of each frame.
		void Update();

		Entity& AddEntity(const std::string& tag);

		EntityVector& GetEntities();
		EntityVector& GetEntities(const std::string& tag);

	private:
		EntityVector m_Entities;
		EntityMap m_EntityMap;

		EntityVector m_ToAdd;

		U64 m_TotalEntites = 0;

	};
}