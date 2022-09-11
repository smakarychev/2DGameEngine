#include "enginepch.h"
#include "ParticleContact.h"

namespace Engine
{
    void ParticleContact::Resolve(F32 duration)
    {
        ResolveVelocity(duration);
        ResolveInterpenetration(duration);
    }

    F32 ParticleContact::CalculateSeparatingVelocity() const
    {
        glm::vec3 relativeVelocity = Particles[0]->GetVelocity();
        if (Particles[1]) relativeVelocity -= Particles[1]->GetVelocity();
        return glm::dot(relativeVelocity, ContactNormal);
    }

    void ParticleContact::ResolveVelocity(F32 duration)
    {
        // Find the velocity in the direction of the contact.
        F32 separatingVel = CalculateSeparatingVelocity();

        // Check whether it needs to be resolved.
        if (separatingVel > 0) return;

        // Calculate the new separating velocity.
        F32 newSeparatingVel = -separatingVel * Restitution;

        // Check the velocity build-up due to acceleration only.
        glm::vec3 velCausedByAcc = Particles[0]->GetAcceleration();
        if (Particles[1]) velCausedByAcc -= Particles[1]->GetAcceleration();
        F32 separatingVelCauseByAcc = glm::dot(velCausedByAcc, ContactNormal) * duration;

        // If we’ve got a closing velocity due to acceleration build-up,
        // remove it from the new separating velocity.
        if (separatingVelCauseByAcc < 0)
        {
            newSeparatingVel += Restitution * separatingVelCauseByAcc;
            if (newSeparatingVel < 0) newSeparatingVel = 0.0f;
        }

        F32 deltaSeparatingVel = newSeparatingVel - separatingVel;
        
        // We apply the change in velocity to each object in proportion to
        // its inverse mass (i.e., those with lower inverse mass [higher
        // actual mass] get less change in velocity).
        F32 totalInverseMass = Particles[0]->GetInverseMass();
        if (Particles[1]) totalInverseMass += Particles[1]->GetInverseMass();
        
        // If all particles have infinite mass, then impulses have no effect.
        if (totalInverseMass <= 0) return;

        // Calculate the impulse to apply.
        F32 impulseMag = deltaSeparatingVel / totalInverseMass;

        // Find the amount of impulse per unit of inverse mass.
        glm::vec3 impulsePerInvMass = ContactNormal * impulseMag;

        // Apply impulses: they are applied in the direction of the contact,
        // and are proportional to the inverse mass.
        Particles[0]->SetVelocity(Particles[0]->GetVelocity() + impulsePerInvMass * Particles[0]->GetInverseMass());
        // Particles[1]'s impulse is in opposite direction (if Particles[1] is present).
        if (Particles[1]) Particles[1]->SetVelocity(Particles[1]->GetVelocity() - impulsePerInvMass * Particles[1]->GetInverseMass());
    }
    
    void ParticleContact::ResolveInterpenetration(F32 duration)
    {
        if (PenetrationDepth <= 0.0f) return;

        // Move each object relative to its mass.
        F32 totalInverseMass = Particles[0]->GetInverseMass();
        if (Particles[1]) totalInverseMass += Particles[1]->GetInverseMass();

        // If all particles have infinite mass, nothing can be done.
        if (totalInverseMass <= 0) return;

        // Find the amount of penetration resolution per unit of inverse mass.
        glm::vec3 movePerInvMass = ContactNormal * (PenetrationDepth / totalInverseMass);

        // Apply the penetration resolution.
        Particles[0]->SetPosition(Particles[0]->GetPosition() + movePerInvMass * Particles[0]->GetInverseMass());
        // Particles[1]'s resolution is in opposite direction (if Particles[1] is present).
        if (Particles[1]) Particles[1]->SetPosition(Particles[1]->GetPosition() - movePerInvMass * Particles[1]->GetInverseMass());
    }
    
    ParticleContactResolver::ParticleContactResolver(U32 maxIterations) :
        m_MaxIterations(maxIterations), m_CurrentIterations(0)
    {
    }

    void ParticleContactResolver::Resolve(std::vector<ParticleContact>& contacts, F32 duration)
    {
        m_CurrentIterations = 0;
        while(m_CurrentIterations < m_MaxIterations)
        {
            // Find the contact with the largest closing velocity.
            F32 maxVel = 0.0f; 
            U32 maxIndex = 0;
            for (U32 i = 0; i < contacts.size(); i++)
            {
                auto& contact = contacts[i];
                F32 separatingVel = contact.CalculateSeparatingVelocity();
                if (separatingVel < maxVel)
                {
                    maxVel = separatingVel;
                    maxIndex = i;
                }
            }
            // Resolve this contact.
            contacts[maxIndex].Resolve(duration);
            m_CurrentIterations++;
        }
    }
}


