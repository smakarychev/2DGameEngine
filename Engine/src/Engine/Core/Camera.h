#pragma once

#include "Engine/Core/Types.h"

#include "Engine/Memory/MemoryManager.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/KeyboardEvents.h"
#include "Engine/Events/MouseEvents.h"
#include "Engine/Events/WindowEvents.h"

#include <memory>
#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;
	// TODO: Rebuild with ECS?
	class Camera
	{
	public:
		enum class ProjectionType
		{
			Perspective, Orthographic
		};
	public:
		Camera();
		Camera(const glm::vec3& position, F32 fov, F32 aspect);

		void SetPosition(const glm::vec3& position);
		void SetFrontDirection(const glm::vec3 direction);

		const glm::vec3& GetFrontDirection() const { return m_Front; }
		const glm::vec3& GetRightDirection() const { return m_Right; }
		const glm::vec3& GetPostion() const { return m_Position; }

		static std::shared_ptr<Camera> Create();
		static std::shared_ptr<Camera> Create(const glm::vec3& position, F32 fov, F32 aspect);

		glm::mat4& GetViewProjection() { return m_ViewProjection; }
	private:
		void UpdateViewMatrix();
		void UpdateOrientationVectors();
	private:
		ProjectionType m_ProjectionType;
		F32 m_Aspect;
		F32 m_NearClipPlane, m_FarClipPlane;
		F32 m_FieldOfView;
		glm::vec3 m_Up, m_Right, m_Front;
		glm::vec3 m_Position;

		glm::mat4 m_ViewProjection;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;


		// TODO: Move to config.
		static const glm::vec3 DEFAULT_POSITION;
		static const glm::vec3 DEFAULT_UP, DEFAULT_FRONT, DEFAULT_RIGHT;
		static const F32 DEFAULT_FOV, DEFAULT_ASPECT;
		static const F32 DEFAULT_NEAR, DEFAULT_FAR;
		static const ProjectionType DEFAULT_PROJECTION_TYPE;
	};

	class CameraController
	{
	public:
		enum class ControllerType
		{
			FPS
		};
	public:
		virtual void OnUpdate(F32 dt) = 0;
		virtual bool OnEvent(Event& event) = 0;
		virtual std::shared_ptr<Camera> GetCamera() = 0;
		static std::shared_ptr<CameraController> Create(ControllerType type, std::shared_ptr<Camera> camera);
	};

	class FPSCameraController : public CameraController
	{
	public:
		FPSCameraController(std::shared_ptr<Camera> camera);

		void SetTranslationSpeed(F32 speed) { m_TranslationSpeed = speed; }
		void SetRotationSpeed(F32 speed) { m_RotationSpeed = speed; }

		void OnUpdate(F32 dt) override;
		bool OnEvent(Event& event) override;

		void SetMouseMoved(bool value) { m_MouseMoved = value; }

		std::shared_ptr<Camera> GetCamera() override { return m_Camera; }
	private:
		glm::vec3 GetNewFrontVector(F32 xOffset, F32 yOffset);
	private:
		std::shared_ptr<Camera> m_Camera;
		F32 m_TranslationSpeed;
		F32 m_RotationSpeed;

		F32 m_Yaw, m_Pitch, m_Roll;

		glm::vec2 m_MouseCoords;

		bool m_MouseMoved = false;

		// TODO: Move to config.
		static const F32 DEFAULT_TRANSLATION_SPEED, DEFAULT_ROTATION_SPEED;
		// E for Euler (DEFAULT_PITCH is some microsoft macro).
		static const F32 DEFAULT_E_YAW, DEFAULT_E_PITCH, DEFAULT_E_ROLL;
	};

}