#include "enginepch.h"

#include "ParticleLinks.h"

#include "Engine/Math/MathUtils.h"

#include <glm/gtx/norm.hpp>

namespace Engine
{
	F32 ParticleLink::CurrentLengthSquared() const
	{
		return glm::distance2(Particles[0]->GetPosition(), Particles[1]->GetPosition());
	}

	U32 ParticleCable::FillContact(std::vector<ParticleContact>& contacts, U32 contactsLimit) const
	{
		F32 length = Math::Sqrt(CurrentLengthSquared());
		if (length < MaxLength) return 0;

		ParticleContact newContact;
		newContact.Particles[0] = Particles[0];
		newContact.Particles[1] = Particles[1];

		glm::vec3 normal = Particles[1]->GetPosition() - Particles[0]->GetPosition();
		normal = glm::normalize(normal);
		newContact.ContactNormal = normal;
		
		newContact.PenetrationDepth = length - MaxLength;
		newContact.Restitution = Restitution;

		contacts.push_back(newContact);
		return 1;
	}

	U32 ParticleRod::FillContact(std::vector<ParticleContact>& contacts, U32 contactsLimit) const
	{
		F32 currentLength = Math::Sqrt(CurrentLengthSquared());
		if (Math::CompareEqual(currentLength, Length)) return 0;

		ParticleContact newContact;
		newContact.Particles[0] = Particles[0];
		newContact.Particles[1] = Particles[1];

		glm::vec3 normal = Particles[1]->GetPosition() - Particles[0]->GetPosition();
		normal = glm::normalize(normal);

		if (currentLength > Length)
		{
			newContact.ContactNormal = normal;
			newContact.PenetrationDepth = currentLength - Length;
		}
		else
		{
			newContact.ContactNormal = -normal;
			newContact.PenetrationDepth = Length - currentLength;
		}
		newContact.Restitution = 0.0f;

		contacts.push_back(newContact);
		return 1;
	}
}

