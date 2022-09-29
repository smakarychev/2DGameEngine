#include "enginepch.h"

#include "RigidBodyForceGenerator.h"

#include "Engine/Math/MathUtils.h"

namespace Engine
{

	Drag2D::Drag2D(F32 k1, F32 k2) 
		: m_K1(k1), m_K2(k2)
	{
	}

	void Drag2D::ApplyForce(RigidBody2D& body, F32 duration)
	{
		glm::vec2 force = body.GetLinearVelocity();
		F32 dragCoefficient = glm::length(force);
		if (dragCoefficient != 0.0f) force = glm::normalize(force);
		dragCoefficient = m_K1 * dragCoefficient + m_K2 * dragCoefficient * dragCoefficient;

		force *= -dragCoefficient;
		body.AddForce(force);
	}

	Spring2D::Spring2D(RigidBody2D* other, const glm::vec2& connection, const glm::vec2& otherConnection, F32 springConstant, F32 restLen)
		: m_Other(other), m_ConnectionPoint(connection), m_OtherConnectionPoint(otherConnection), m_SpringConstant(springConstant), m_RestLength(restLen)
	{
	}

	void Spring2D::ApplyForce(RigidBody2D& body, F32 duration)
	{
		// Calculate the two ends in world space.
		glm::vec2 springBeg = body.TransformToWorld(m_ConnectionPoint);
		glm::vec2 springEnd = m_Other->TransformToWorld(m_OtherConnectionPoint);
		glm::vec2 force = springBeg - springEnd;
		
		F32 forceMagnitude = glm::length(force);
		if (forceMagnitude != 0.0f) force = glm::normalize(force);

		forceMagnitude = Math::Abs(forceMagnitude - m_RestLength);
		forceMagnitude *= m_SpringConstant;

		force *= -forceMagnitude;
		body.ApplyForce(force, springBeg);
	}
	
	Aero2D::Aero2D(const glm::mat2& aeroTensor, const glm::vec2& position, glm::vec2* windSpeed)
		: m_Tensor(aeroTensor), m_Position(position), m_WindSpeed(windSpeed)
	{
	}

	void Aero2D::ApplyForce(RigidBody2D& body, F32 duration)
	{
		ApplyForceFromTensor(body, duration, m_Tensor);
	}

	void Aero2D::ApplyForceFromTensor(RigidBody2D& body, F32 duration, const glm::mat2& tensor)
	{
		glm::vec2 velocity = body.GetLinearVelocity();
		if (m_WindSpeed != nullptr) velocity += *m_WindSpeed;

		// Translate velocity to local coordinates.
		velocity = body.TransformDirectionToLocal(velocity);
		
		glm::vec2 forceLocal = tensor * velocity;
		glm::vec2 force = body.TransformDirectionToWorld(forceLocal);

		body.ApplyForceLocal(force, m_Position);
	}

	void RigidBody2DForceRegistry::Add(Ref<RigidBody2DForceGenerator> generator, RigidBody2D* body)
	{
		auto it = m_Registry.find(generator);
		if (it != m_Registry.end()) m_Registry[generator].push_back(body);
		else m_Registry.emplace(generator, std::vector<RigidBody2D*>({ body }));
	}
	
	void RigidBody2DForceRegistry::Remove(Ref<RigidBody2DForceGenerator> generator, RigidBody2D* body)
	{
		auto it = m_Registry.find(generator);
		if (it != m_Registry.end())
		{
			auto elemIt = std::find(it->second.begin(), it->second.end(), body);
			if (elemIt != it->second.end())	it->second.erase(elemIt);
		}
	}
	
	void RigidBody2DForceRegistry::Clear()
	{
		m_Registry.clear();
	}
	
	void RigidBody2DForceRegistry::ApplyForces(F32 duration)
	{
		for (auto&& [generator, bodies] : m_Registry)
		{
			for (auto& body : bodies)
			{
				generator->ApplyForce(*body, duration);
			}
		}
	}
}


