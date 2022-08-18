#pragma once

#include "Engine/Core/Types.h"

#include "Engine/Memory/MemoryManager.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/KeyboardEvents.h"
#include "Engine/Events/MouseEvents.h"
#include "Engine/Events/WindowEvents.h"

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Engine
{
	using namespace Types;
	// TODO: Rebuild with ECS?
	class Camera
	{
		friend class EditorCameraController;
		friend class Editor2DCameraController;
		friend class FPSCameraController;
	public:
		enum class ProjectionType
		{
			Perspective, Orthographic
		};
	public:
		static std::shared_ptr<Camera> Create();
		static std::shared_ptr<Camera> Create(const glm::vec3& position, F32 fov, F32 aspect);
		
		Camera();
		Camera(const glm::vec3& position, F32 fov, F32 aspect);

		void SetPosition(const glm::vec3& position);
		void SetOrientation(const glm::quat& orientation);

		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::quat& GetOrientation() const { return m_Orientation; }

		glm::mat4& GetViewProjection() { return m_ViewProjection; }

		void SetProjection(ProjectionType type);
		void SetViewport(U32 width, U32 height);

		glm::vec3 GetForward() const;
		glm::vec3 GetUp() const;
		glm::vec3 GetRight() const;

		glm::vec2 ScreenToWorldPoint(const glm::vec2& screenPosition) const;

	private:
		void UpdateViewMatrix();
		void UpdateProjectionMatrix();
		void UpdateViewProjection();
	private:
		ProjectionType m_ProjectionType;
		F32 m_Aspect;
		F32 m_NearClipPlane, m_FarClipPlane;
		F32 m_FieldOfView;

		U32 m_ViewportWidth, m_ViewportHeight;

		glm::vec3 m_Position;
		glm::quat m_Orientation;

		glm::mat4 m_ViewProjection;
		glm::mat4 m_ViewProjectionInverse;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;

		// This should not have DEFAULT_...
		F32 m_OrthoZoom = 1.0f;


		// TODO: Move to config.
		static const glm::vec3 DEFAULT_POSITION;
		static const glm::quat DEFAULT_ORIENTATION;
		static const F32 DEFAULT_FOV, DEFAULT_ASPECT;
		static const F32 DEFAULT_NEAR, DEFAULT_FAR;
		static const ProjectionType DEFAULT_PROJECTION_TYPE;
	};

	class CameraController
	{
	public:
		enum class ControllerType
		{
			FPS, Editor, Editor2D
		};
	public:
		static std::shared_ptr<CameraController> Create(ControllerType type, std::shared_ptr<Camera> camera);

		virtual void OnUpdate(F32 dt) = 0;
		virtual bool OnEvent(Event& event) = 0;
		virtual std::shared_ptr<Camera> GetCamera() = 0;
	};

	class FPSCameraController : public CameraController
	{
	public:
		FPSCameraController(std::shared_ptr<Camera> camera);

		void OnUpdate(F32 dt) override;
		bool OnEvent(Event& event) override;
		std::shared_ptr<Camera> GetCamera() override { return m_Camera; }

		void SetTranslationSpeed(F32 speed) { m_TranslationSpeed = speed; }
		void SetMousemSensitivity(F32 sensitivity) { m_MouseSensitivity = sensitivity; }
	private:
		std::shared_ptr<Camera> m_Camera;
		F32 m_TranslationSpeed;
		F32 m_MouseSensitivity;

		F32 m_Yaw, m_Pitch;

		glm::vec2 m_MouseCoords;

		// TODO: Move to config.
		static const F32 DEFAULT_TRANSLATION_SPEED, DEFAULT_SENSITIVITY;
		// E for Euler (DEFAULT_PITCH is some microsoft macro).
		static const F32 DEFAULT_E_YAW, DEFAULT_E_PITCH;
	};

	class EditorCameraController : public CameraController
	{
	public:
		EditorCameraController(std::shared_ptr<Camera> camera);

		void OnUpdate(F32 dt) override;
		bool OnEvent(Event& event) override;
		std::shared_ptr<Camera> GetCamera() { return m_Camera; };

		void SetTranslationSpeed(F32 speed) { m_TranslationSpeed = speed; }
		void SetRotationSpeed(F32 speed) { m_RotationSpeed = speed; }
	protected:
		F32 ZoomSpeed();
	protected:
		std::shared_ptr<Camera> m_Camera;

		F32 m_TranslationSpeed, m_RotationSpeed;
		F32 m_Yaw, m_Pitch;
		glm::vec2 m_MouseCoords;

		glm::vec3 m_FocalPoint;
		F32 m_Distance;

		bool m_AnglesConstrained = false;

		// TODO: Move to config.
		static const F32 DEFAULT_TRANSLATION_SPEED, DEFAULT_ROTATION_SPEED;
		// E for Euler (DEFAULT_PITCH is some microsoft macro).
		static const F32 DEFAULT_E_YAW, DEFAULT_E_PITCH;
		static const F32 DEFAULT_DISTANCE;
		static const glm::vec3 DEFAULT_FOCAL_POINT;
	};
	class Editor2DCameraController : public EditorCameraController
	{
	public:
		Editor2DCameraController(std::shared_ptr<Camera> camera) : EditorCameraController(camera)
		{
			m_AnglesConstrained = true;
			camera->SetProjection(Camera::ProjectionType::Orthographic);
		}
	};
}