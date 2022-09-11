#include "enginepch.h"
#include "ParticleForceGenerator.h"

#include "Engine/Math/MathUtils.h"

namespace Engine
{
	ParticleDrag::ParticleDrag(F32 k1, F32 k2) :
		m_K1(k1), m_K2(k2)
	{
	}
	
	void ParticleDrag::ApplyForce(Particle& particle, F32 duration)
	{
		glm::vec3 force = particle.GetVelocity();
		F32 dragCoefficient = glm::length(force);
		if (dragCoefficient != 0.0f) force = glm::normalize(force);
		dragCoefficient = m_K1 * dragCoefficient + m_K2 * dragCoefficient * dragCoefficient;

		force *= -dragCoefficient;
		particle.AddForce(force);
	}

	ParticleSpring::ParticleSpring(Particle* other, F32 coeff, F32 restLength) :
		m_Other(other), m_SpringConstant(coeff), m_RestLength(restLength)
	{
	}

	void ParticleSpring::ApplyForce(Particle& particle, F32 duration)
	{
		glm::vec3 force = particle.GetPosition();
		force -= m_Other->GetPosition();
		F32 forceMagnitude = glm::length(force);
		if (forceMagnitude != 0.0f) force = glm::normalize(force);
		forceMagnitude = Math::Abs(forceMagnitude - m_RestLength);
		forceMagnitude *= m_SpringConstant;

		force *= -forceMagnitude;
		particle.AddForce(force);
	}

	ParticleAnchoredSpring::ParticleAnchoredSpring(const glm::vec3& anchor, F32 coeff, F32 restLength) :
		m_Anchor(anchor), m_SpringConstant(coeff), m_RestLength(restLength)
	{
	}
	
	void ParticleAnchoredSpring::ApplyForce(Particle& particle, F32 duration)
	{
		glm::vec3 force = particle.GetPosition();
		force -= m_Anchor;
		F32 forceMagnitude = glm::length(force);
		if (forceMagnitude != 0.0f) force = glm::normalize(force);
		forceMagnitude = Math::Abs(forceMagnitude - m_RestLength);
		forceMagnitude *= m_SpringConstant;

		force *= -forceMagnitude;
		particle.AddForce(force);
	}

	ParticleBungee::ParticleBungee(Particle* other, F32 coeff, F32 restLength) :
		m_Other(other), m_SpringConstant(coeff), m_RestLength(restLength)
	{
	}
	
	void ParticleBungee::ApplyForce(Particle& particle, F32 duration)
	{
		glm::vec3 force = particle.GetPosition();
		force -= m_Other->GetPosition();
		F32 forceMagnitude = glm::length(force);
		if (forceMagnitude < m_RestLength) return;
		if (forceMagnitude != 0.0f) force = glm::normalize(force);
		forceMagnitude = Math::Abs(forceMagnitude - m_RestLength);
		forceMagnitude *= m_SpringConstant;

		force *= -forceMagnitude;
		particle.AddForce(force);
	}

	ParticleBuoyancy::ParticleBuoyancy(F32 maxDepth, F32 volume, F32 waterHeight, F32 liquidDensity) :
		m_MaxDepth(maxDepth), m_Volume(volume), m_WaterHeight(waterHeight), m_LiquidDensity(liquidDensity)
	{
	}
	
	void ParticleBuoyancy::ApplyForce(Particle& particle, F32 duration)
	{
		F32 particleDepth = particle.GetPosition().y;

		// If above liqud.
		if (particleDepth - m_MaxDepth >= m_WaterHeight) return;
		glm::vec3 force = glm::vec3{ 0.0f };

		// If completely submerged.
		if (particleDepth + m_MaxDepth <= m_WaterHeight)
		{
			force.y = m_LiquidDensity * m_Volume;
			particle.AddForce(force);
			return;
		}

		// Else the particle is only partially submerged.
		force.y = m_LiquidDensity * m_Volume * (particleDepth - m_MaxDepth - m_WaterHeight) / (m_MaxDepth * 2.0f);
		particle.AddForce(force);
	}

	void ParticleForceRegistry::Add(Ref<ParticleForceGenerator> generator, Particle* particle)
	{
		auto it = m_Registry.find(generator);
		if (it != m_Registry.end()) m_Registry[generator].push_back(particle);
		else m_Registry.emplace(generator, std::vector<Particle*>({ particle }));
	}

	void ParticleForceRegistry::Remove(Ref<ParticleForceGenerator> generator, Particle* particle)
	{
		auto it = m_Registry.find(generator);
		if (it != m_Registry.end())
		{
			auto elemIt = std::find(it->second.begin(), it->second.end(), particle);
			if (elemIt != it->second.end())	it->second.erase(elemIt);
		}
	}
	
	void ParticleForceRegistry::Clear()
	{
		m_Registry.clear();
	}
	
	void ParticleForceRegistry::ApplyForces(F32 duration)
	{
		for (auto&& [generator, particles] : m_Registry)
		{
			for (auto& particle : particles)
			{
				generator->ApplyForce(*particle, duration);
			}		
		}
	}

}

