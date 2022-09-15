#pragma once

#include "Engine/Core/Types.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;
	
	class Particle
	{
	public:
		Particle(const glm::vec3& position = glm::vec3{ 1.0f }, F32 mass = 1.0f);
		
		void SetPosition(const glm::vec3& position) { m_Position = position; }
		const glm::vec3& GetPosition() const { return m_Position; }
		
		void SetVelocity(const glm::vec3& velocity) { m_Velocity = velocity; }
		const glm::vec3& GetVelocity() const { return m_Velocity; }
		
		void SetAcceleration(const glm::vec3& acceleration) { m_Acceleration = acceleration; }
		const glm::vec3& GetAcceleration() const { return m_Acceleration; }
		
		void SetMass(F32 mass) { m_InverseMass = 1.0f / mass; }
		void SetInverseMass(F32 invMass) { m_InverseMass = invMass; }
		F32 GetMass() const { return 1.0f / m_InverseMass; }
		F32 GetInverseMass() const { return m_InverseMass; }
		bool HasFiniteMass() const { return m_InverseMass != 0.0f; }
		void SetInfiniteMass() { m_InverseMass = 0.0f; }

		void SetDamping(F32 damping) { m_Damping = damping; }
		F32 GetDamping() const { return m_Damping; }

		void AddForce(const glm::vec3& force) { m_Force += force; }
		void ResetForce() { m_Force = glm::vec3{ 0.0f }; }
		const glm::vec3& GetForce() const { return m_Force; }


	private:
		glm::vec3 m_Position;
		glm::vec3 m_Velocity;
		glm::vec3 m_Acceleration;

		glm::vec3 m_Force;

		F32 m_InverseMass;
		// Acts like a simple form of drag force, helps to deal with numerical inaccuracies.
		F32 m_Damping;
	};
}
