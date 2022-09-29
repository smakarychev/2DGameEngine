#pragma once

#include "RigidBody.h"
#include "RigidBodyForceGenerator.h"

#include "Engine/Core/Core.h"

#include <vector>


namespace Engine
{
	using namespace Types;

	class RigidBody2DWorld
	{
	public:
		RigidBody2DWorld(const glm::vec2& gravity = glm::vec2{ 0.0f, -10.0f });

		// All rigid bodies shall be created by this method.
		RigidBody2D& CreateBody(glm::vec3 position = glm::vec3{ 0.0f }, F32 mass = 1.0f, F32 inertia = 1.0f);
		
		void Update(F32 deltaTime);

		// Add the global force, acting on all bodies.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator);

		// Add force to some body.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator, RigidBody2D& body);

		void SetGravity(const glm::vec2& gravity) { m_Gravity = gravity; }
		const std::vector<Ref<RigidBody2D>>& GetBodies() const { return m_Bodies; }
	private:
		void VelocityVerletIntegration(F32 deltaTime);
		void ApplyGlobalForces();
	private:
		// Stores all the bodies (smart poitners to them).
		std::vector<Ref<RigidBody2D>> m_Bodies;

		// Stores all global forces.
		std::vector<Ref<RigidBody2DForceGenerator>> m_GlobalForces;

		// Stores all force generators and the bodies they apply force to.
		RigidBody2DForceRegistry m_Registry;

		glm::vec2 m_Gravity;

	};
}