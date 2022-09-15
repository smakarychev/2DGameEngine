#pragma once

#include "Particle.h"
#include "ParticleForceGenerator.h"
#include "ParticleContact.h"
#include "ParticleLinks.h"

#include <vector>

namespace Engine
{
	using namespace Types;

	class ParticleWorld
	{
	public:
		ParticleWorld(const glm::vec3& gravity = glm::vec3{ 0.0f, -10.0f, 0.0f });

		// All particles shall be created by this method.
		Particle& CreateParticle(glm::vec3 position = glm::vec3{ 0.0f }, F32 mass = 1.0f);

		ParticleCable& CreateParticleCable(Particle& first, Particle& second, F32 maxLength, F32 restitution = 1.0f);
		ParticleRod& CreateParticleRod(Particle& first, Particle& second, F32 length);

		// Integrate all particles.
		void Update(F32 deltaTime);

		// Add the global force, acting on all particles.
		void AddForce(Ref<ParticleForceGenerator> forceGenerator);
		
		// Add force to some particle.
		void AddForce(Ref<ParticleForceGenerator> forceGenerator, Particle& particle);

		const std::vector<Ref<Particle>>& GetParticles() const { return m_Particles; }
	private:
		void VelocityVerletIntegration(F32 deltaTime);
		void ApplyGlobalForces();
		void ProcessLinks(F32 deltaTime);
	private:
		// Particles that are updated every frame.
		std::vector<Ref<Particle>> m_Particles;

		// Stores all force generators and the particles they apply force to.
		ParticleForceRegistry m_Registry;

		// Stores all global forces.
		std::vector<Ref<ParticleForceGenerator>> m_GlobalForces;

		// Gravity force that acts on all particles.
		glm::vec3 m_Gravity;

		ParticleContactResolver m_ContactResolver;
		std::vector<ParticleContact> m_ParticleContacts;
		std::vector<Ref<ParticleLink>> m_ParticleLinks;
	};

}