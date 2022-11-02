#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Core.h"

#include "Engine/ECS/Components.h"

#include <memory>
#include <tuple>

namespace Engine
{
	using namespace Types;
	// Everything will probably change.
	struct Entity
	{
		using ComponentMap = std::unordered_map<U64, bool>;
		
		friend class EntityManager;
		FRIEND_MEMORY_FN
		void Destroy() { IsActive = false; }
		
		U64 Id = 0;
		std::string Tag = "Default";
		bool IsActive = true;

		template <typename T>
		bool HasComponent();
		template <typename T, typename R, typename  ... Others>
		bool HasComponent();
		template <typename T, typename R, typename  ... Others>
		bool HasAnyComponent();
		// User should firstly check that such component exits.
		template <typename T>
		T& GetComponent();
		template <typename T>
		const T& GetComponent() const;
		template <typename T>
		void RemoveComponent();
		template <typename T, typename ... Args>
		T& AddComponent(Args&&... args);
		
	private:
		Entity(std::string tag, U64 id) :
			Id(id), Tag(std::move(tag)) {}
	private:
		Component::Components m_Components{};
		ComponentMap m_ComponentMap{};
	};

	// TODO: find a better way.
	template <typename T>
	bool Entity::HasComponent()
	{
		const U64 id = typeid(T).hash_code();
		// [] operator will default to false if no such key in the map.
		return m_ComponentMap[id];
	}

	template <typename T, typename R, typename ... Others>
	bool Entity::HasComponent()
	{
		U64 id = typeid(T).hash_code();
		bool has = m_ComponentMap[id];
		id = typeid(R).hash_code();
		if (!has) return has;
		return has &= HasComponent<R, Others...>();
	}

	template <typename T, typename R, typename  ... Others>
	bool Entity::HasAnyComponent()
	{
		U64 id = typeid(T).hash_code();
		bool has = m_ComponentMap[id];
		id = typeid(R).hash_code();
		if (has) return has;
		return has |= HasAnyComponent<R, Others...>();
	}

	template <typename T>
	T& Entity::GetComponent()
	{
		return std::get<T>(m_Components);
	}

	template <typename T>
	const T& Entity::GetComponent() const
	{
		return std::get<T>(m_Components);
	}

	template <typename T>
	void Entity::RemoveComponent()
	{
		const U64 id = typeid(T).hash_code();
		m_ComponentMap[id] = false;
	}

	template <typename T, typename ... Args>
	T& Entity::AddComponent(Args&&... args)
	{
		auto& component = std::get<T>(m_Components);
		component = T(std::forward<Args>(args)...);
		const U64 id = typeid(T).hash_code();
		m_ComponentMap[id] = true;
		return component;
	}
}
