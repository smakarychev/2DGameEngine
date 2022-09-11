#pragma once

#include "Engine/Core/Types.h"

#include "Particle.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;
	
	class ParticleContact
	{
	public:
		void Resolve(F32 duration);
		F32 CalculateSeparatingVelocity() const;
	private:
		void ResolveVelocity(F32 duration);
		void ResolveInterpenetration(F32 duration);
	public:
		// Colliding particles (2nd is nullptr if it is collision with wall).
		Particle* Particles[2];

		F32 Restitution;

		glm::vec3 ContactNormal;

		// Depth of interpenetration in the direction of the contant (ContactNormal).
		F32 PenetrationDepth;
	};

	class ParticleContactResolver
	{
	public:
		ParticleContactResolver(U32 maxIterations = 0);
		void Resolve(std::vector<ParticleContact>& contacts, F32 duration);
		void SetIterations(U32 iters) { m_MaxIterations = iters; }
	private:
		U32 m_MaxIterations;
		U32 m_CurrentIterations;
	};
}