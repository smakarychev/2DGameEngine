#pragma once

#include "Engine/Core/Types.h"

#include "ParticleContact.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;
	// Used as a base class for cables and rods, and could be used as a base
	// class for springs with a limit to their extension.
	class ParticleLink
	{
	public:
		// The new contact is pushed back to the vector of contacts,
		// where limit is the maximum number of contacts in the array that can be written to (0 if unlimited). 
		// The method returns the number of contacts that have been written.
		virtual U32 FillContact(std::vector<ParticleContact>& contacts, U32 contactsLimit) const = 0;
	protected:
		// Returns squared length of the rod / cable or smth else.
		F32 CurrentLengthSquared() const;
	public:
		Particle* Particles[2] = { nullptr, nullptr };
	};

	class ParticleCable : public ParticleLink
	{
	public:
		U32 FillContact(std::vector<ParticleContact>& contacts, U32 contactsLimit) const override;
	public:
		F32 MaxLength = 0.0f;
		F32 Restitution = 0.0f;
	};

	class ParticleRod : public ParticleLink
	{
	public:
		U32 FillContact(std::vector<ParticleContact>& contacts, U32 contactsLimit) const override;
	public:
		F32 Length = 0.0f;
	};
}
