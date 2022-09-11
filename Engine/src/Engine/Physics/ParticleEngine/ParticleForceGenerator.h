#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

#include "Particle.h"

#include <glm/glm.hpp>
#include <unordered_map>

namespace Engine
{
	using namespace Types;

	class ParticleForceGenerator
	{
	public:
		virtual void ApplyForce(Particle& particle, F32 duration = 0.0f) = 0;
		virtual ~ParticleForceGenerator() {}
	};

	class ParticleDrag : public ParticleForceGenerator
	{
	public:
		ParticleDrag(F32 k1, F32 k2);
		void ApplyForce(Particle& particle, F32 duration = 0.0f) override;
	private:
		F32 m_K1, m_K2;
	};

	class ParticleSpring : public ParticleForceGenerator
	{
	public:
		ParticleSpring(Particle* other, F32 coeff, F32 restLength);
		void ApplyForce(Particle& particle, F32 duration = 0.0f) override;
	private:
		Particle* m_Other;
		F32 m_RestLength;
		F32 m_SpringConstant;
	};

	class ParticleAnchoredSpring : public ParticleForceGenerator
	{
	public:
		ParticleAnchoredSpring(const glm::vec3& anchor, F32 coeff, F32 restLength);
		void ApplyForce(Particle& particle, F32 duration = 0.0f) override;
	private:
		glm::vec3 m_Anchor;
		F32 m_RestLength;
		F32 m_SpringConstant;
	};

	// Same as spring, but acts only when extended.
	class ParticleBungee : public ParticleForceGenerator
	{
	public:
		ParticleBungee(Particle* other, F32 coeff, F32 restLength);
		void ApplyForce(Particle& particle, F32 duration = 0.0f) override;
	private:
		Particle* m_Other;
		F32 m_RestLength;
		F32 m_SpringConstant;
	};

	class ParticleBuoyancy : public ParticleForceGenerator
	{
	public:
		ParticleBuoyancy(F32 maxDepth, F32 volume, F32 waterHeight, F32 liquidDensity = 1000.0f);
		void ApplyForce(Particle& particle, F32 duration = 0.0f) override;
	private:
		F32 m_MaxDepth;
		F32 m_Volume;
		F32 m_WaterHeight;
		F32 m_LiquidDensity;
	};

	class ParticleForceRegistry
	{
	public:
		void Add(Ref<ParticleForceGenerator> generator, Particle* particle);
		void Remove(Ref<ParticleForceGenerator> generator, Particle* particle);
		void Clear();

		void ApplyForces(F32 duration = 1.0f);

		const auto& GetRegistry() const { return m_Registry; }
	private:
		std::unordered_map<Ref<ParticleForceGenerator>, std::vector<Particle*>> m_Registry;
	};
}