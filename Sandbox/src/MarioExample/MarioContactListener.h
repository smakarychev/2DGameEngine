#pragma once

#include "Engine.h"
#include "Engine/ECS/Registry.h"

using namespace Engine;
using namespace Engine::Types;

struct CollisionCallback
{
	struct CollisionData
	{
		Entity Primary{NULL_ENTITY};
		Entity Secondary{NULL_ENTITY};
		Physics::ContactListener::ContactState ContactState{Physics::ContactListener::ContactState::Begin};
	};
	using SensorCallback = void (*)(Registry* registry, const CollisionData& collisionData,
									[[maybe_unused]] const Physics::ContactInfo2D& contact);
	std::string IndexMajor{};
	std::string IndexMinor{};
	U32 CollisionCount{0};
};

class MarioContactListener : public Physics::ContactListener
{
public:
	void SetRegistry(Registry* registry) { m_Registry = registry; }
	void SetSensorCallbacks(std::unordered_map<std::string, std::unordered_map<std::string, CollisionCallback::SensorCallback>>* callbacks) { m_SensorCallbacks = callbacks; }
	void OnContactBegin([[maybe_unused]] const Physics::ContactInfo2D& contact) override;
	void OnContactEnd([[maybe_unused]] const Physics::ContactInfo2D& contact) override;
private:
	void PropagateToCallback(const Physics::ContactInfo2D& contact, ContactState state);
private:
	Registry* m_Registry = nullptr;
	std::unordered_map<std::string, std::unordered_map<std::string, CollisionCallback::SensorCallback>>* m_SensorCallbacks{nullptr};
};