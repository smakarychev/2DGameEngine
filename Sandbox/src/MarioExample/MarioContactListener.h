#pragma once

#include "Engine.h"
#include "Engine/ECS/Registry.h"

using namespace Engine;
using namespace Engine::Types;

class MarioContactListener : public Physics::ContactListener
{
public:
	void SetRegistry(Registry* registry) { m_Registry = registry; }
	void OnContactBegin([[maybe_unused]] const Physics::ContactInfo2D& contact) override;
	void OnContactEnd([[maybe_unused]] const Physics::ContactInfo2D& contact) override;
private:
	void PropagateToCallback(const Physics::ContactInfo2D& contact, ContactState state);
private:
	Registry* m_Registry = nullptr;
};