#include "enginepch.h"
#include "Particle.h"

namespace Engine
{
	Particle::Particle(const glm::vec3& position, F32 mass) :
		m_Position(position), m_Velocity(0.0f), m_Acceleration(0.0f), m_Force(0.0f), m_InverseMass(1.0f / mass), m_Damping(1.0f)
	{
	}
}


