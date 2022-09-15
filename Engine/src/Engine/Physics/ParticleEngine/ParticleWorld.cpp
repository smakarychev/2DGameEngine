#include "enginepch.h"
#include "ParticleWorld.h"

#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
    ParticleWorld::ParticleWorld(const glm::vec3& gravity) : m_Gravity(gravity)
    {
    }
    
    Particle& ParticleWorld::CreateParticle(glm::vec3 position, F32 mass)
    {
        m_Particles.emplace_back(CreateRef<Particle>(position, mass));
        return *m_Particles.back();
    }

    ParticleCable& ParticleWorld::CreateParticleCable(Particle& first, Particle& second, F32 maxLength, F32 restitution)
    {
        Ref<ParticleCable> parCable = CreateRef<ParticleCable>();
        parCable->Particles[0] = &first;
        parCable->Particles[1] = &second;
        parCable->Restitution = restitution;
        parCable->MaxLength = maxLength;
        m_ParticleLinks.push_back(parCable);
        return *parCable;
    }

    ParticleRod& ParticleWorld::CreateParticleRod(Particle& first, Particle& second, F32 length)
    {
        Ref<ParticleRod> parRod = CreateRef<ParticleRod>();
        parRod->Particles[0] = &first;
        parRod->Particles[1] = &second;
        parRod->Length = length;
        m_ParticleLinks.push_back(parRod);
        return *parRod;
    }

    void ParticleWorld::Update(F32 deltaTime)
    {
        VelocityVerletIntegration(deltaTime);

        m_ParticleContacts.clear();
        m_ParticleContacts.reserve(128);
        ProcessLinks(deltaTime);
        m_ContactResolver.SetIterations(U32(m_ParticleContacts.size()) * 1);
        m_ContactResolver.Resolve(m_ParticleContacts, deltaTime);
    }

    void ParticleWorld::VelocityVerletIntegration(F32 deltaTime)
    {
        // First update positions.
        for (auto& particle : m_Particles)
        {
            glm::vec3 newPosition = particle->GetPosition() +
                particle->GetVelocity() * deltaTime +
                particle->GetAcceleration() * deltaTime * deltaTime * 0.5f;
            particle->SetPosition(newPosition);
        }
        // Then apply forces.
        ApplyGlobalForces();
        m_Registry.ApplyForces();
        // Then update acceleration and velocity.
        for (auto& particle : m_Particles)
        {
            glm::vec3 newAcceleration = glm::vec3{ 0.0f };
            if (particle->HasFiniteMass())
            {
                newAcceleration += m_Gravity;
                newAcceleration += particle->GetForce() * particle->GetInverseMass();
            }
            glm::vec3 newVelocity = particle->GetVelocity() +
                (particle->GetAcceleration() + newAcceleration) * deltaTime * 0.5f;
            particle->SetVelocity(newVelocity * particle->GetDamping());
            particle->SetAcceleration(newAcceleration);

            particle->ResetForce();
        }
    }

    void ParticleWorld::ApplyGlobalForces()
    {
        for (auto& globalForce : m_GlobalForces)
        {
            for (auto& particle : m_Particles)
            {
                globalForce->ApplyForce(*particle);
            }
        }
    }

    void ParticleWorld::ProcessLinks(F32 deltaTime)
    {
        for (auto& link : m_ParticleLinks)
        {
            link->FillContact(m_ParticleContacts, 0);
        }
    }

    void ParticleWorld::AddForce(Ref<ParticleForceGenerator> forceGenerator)
    {
        m_GlobalForces.push_back(forceGenerator);
    }

    void ParticleWorld::AddForce(Ref<ParticleForceGenerator> forceGenerator, Particle& particle)
    {
        m_Registry.Add(forceGenerator, &particle);
    }


}

