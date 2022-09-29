#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

#include "RigidBody.h"

#include <glm/glm.hpp>
#include <unordered_map>

namespace Engine
{
	using namespace Types;

	class RigidBody2DForceGenerator
	{
	public:
		virtual void ApplyForce(RigidBody2D& body, F32 duration = 0.0f) = 0;
		virtual ~RigidBody2DForceGenerator() {}
	};

	class Drag2D : public RigidBody2DForceGenerator
	{
	public:
		Drag2D(F32 k1, F32 k2);
		void ApplyForce(RigidBody2D& body, F32 duration = 0.0f) override;
	private:
		F32 m_K1, m_K2;
	};

	class Spring2D : public RigidBody2DForceGenerator
	{
	public:
		Spring2D(RigidBody2D* other, const glm::vec2& connection, const glm::vec2& otherConnection, F32 springConstant = 1.0f, F32 restLen = 1.0f);
		void ApplyForce(RigidBody2D& body, F32 duration = 0.0f) override;
	private:
		RigidBody2D* m_Other;
		F32 m_RestLength;
		F32 m_SpringConstant;

		glm::vec2 m_ConnectionPoint;
		glm::vec2 m_OtherConnectionPoint;
	};

	// Aerodynamic force.
	class Aero2D : public RigidBody2DForceGenerator
	{
	public:
		Aero2D(const glm::mat2& aeroTensor, const glm::vec2& position, glm::vec2* windSpeed = nullptr);
		void ApplyForce(RigidBody2D& body, F32 duration = 0.0f) override;
	private:
		void ApplyForceFromTensor(RigidBody2D& body, F32 duration, const glm::mat2& tensor);
	private:
		// Aerodynamic tensor of the surface in local coordinate space.
		glm::mat2 m_Tensor;
		// Position of aerodynamic surface in local coordinate space.
		glm::vec2 m_Position;
		// Pointer to the wind velocity vector, that is potentially changed outside.
		glm::vec2* m_WindSpeed;
	};

	class RigidBody2DForceRegistry
	{
	public:
		void Add(Ref<RigidBody2DForceGenerator> generator, RigidBody2D* body);
		void Remove(Ref<RigidBody2DForceGenerator> generator, RigidBody2D* body);
		void Clear();

		void ApplyForces(F32 duration = 1.0f);

		const auto& GetRegistry() const { return m_Registry; }
	private:
		std::unordered_map<Ref<RigidBody2DForceGenerator>, std::vector<RigidBody2D*>> m_Registry;
	};
}