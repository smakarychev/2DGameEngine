#include "enginepch.h"

#include "Camera.h"

#include "Engine/Core/Input.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Engine
{
	const glm::vec3 Camera::DEFAULT_POSITION	= glm::vec3(0.0f);
	const glm::vec3 Camera::DEFAULT_UP			= glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::vec3 Camera::DEFAULT_FRONT		= glm::vec3(0.0f, 0.0f, -1.0f);
	const glm::vec3 Camera::DEFAULT_RIGHT		= glm::vec3(1.0f, 1.0f, 0.0f);
	const F32  Camera::DEFAULT_FOV				= 45.0f;
	const F32  Camera::DEFAULT_ASPECT			= 16.0f / 9.0f;
	const F32  Camera::DEFAULT_NEAR				= 0.03f;
	const F32  Camera::DEFAULT_FAR				= 1000.0f;
	const Camera::ProjectionType Camera::DEFAULT_PROJECTION_TYPE = Camera::ProjectionType::Perspective;

	std::shared_ptr<Camera> Camera::Create()
	{
		return std::shared_ptr<Camera>(New<Camera>(), Delete<Camera>);
	}

	std::shared_ptr<Camera> Camera::Create(const glm::vec3& position, F32 fov, F32 aspect)
	{
		return std::shared_ptr<Camera>(New<Camera>(position, fov, aspect), Delete<Camera>);
	}

	Camera::Camera() : m_FieldOfView(DEFAULT_FOV), m_Aspect(DEFAULT_ASPECT),
		m_NearClipPlane(DEFAULT_NEAR), m_FarClipPlane(DEFAULT_FAR), m_ProjectionType(DEFAULT_PROJECTION_TYPE),
		m_Position(DEFAULT_POSITION), m_Up(DEFAULT_UP), m_Right(DEFAULT_RIGHT), m_Front(DEFAULT_FRONT)
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		UpdateViewMatrix();
		m_ViewProjection = m_ProjectionMatrix * m_ViewMatrix;
	}

	Camera::Camera(const glm::vec3& position, F32 fov, F32 aspect) : m_FieldOfView(fov), m_Aspect(aspect),
		m_NearClipPlane(DEFAULT_NEAR), m_FarClipPlane(DEFAULT_FAR), m_ProjectionType(DEFAULT_PROJECTION_TYPE),
		m_Position(position), m_Up(DEFAULT_UP), m_Right(DEFAULT_RIGHT), m_Front(DEFAULT_FRONT)
	{
		UpdateViewMatrix();
		switch (m_ProjectionType)
		{
		case ProjectionType::Perspective:
			m_ProjectionMatrix = glm::perspective(m_FieldOfView, m_Aspect, m_NearClipPlane, m_FarClipPlane);
			break;
		case ProjectionType::Orthographic:
			m_ProjectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f * m_Aspect, 1.0f * m_Aspect, m_NearClipPlane, m_FarClipPlane);
			break;
		}
		m_ViewProjection = m_ProjectionMatrix * m_ViewMatrix;
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		UpdateViewMatrix();
		switch (m_ProjectionType)
		{
		case ProjectionType::Perspective:
			m_ProjectionMatrix = glm::perspective(m_FieldOfView, m_Aspect, m_NearClipPlane, m_FarClipPlane);
			break;
		case ProjectionType::Orthographic:
			m_ProjectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f * m_Aspect, 1.0f * m_Aspect, m_NearClipPlane, m_FarClipPlane);
			break;
		}
		m_ViewProjection = m_ProjectionMatrix * m_ViewMatrix;
	}

	void Camera::SetFrontDirection(const glm::vec3 direction)
	{
		m_Front = direction;
		UpdateOrientationVectors();
	}

	void Camera::UpdateViewMatrix()
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
	}

	void Camera::UpdateOrientationVectors()
	{
		// Default up is world up.
		m_Right = glm::normalize(glm::cross(m_Front, DEFAULT_UP));
		m_Up = glm::normalize(glm::cross(m_Right, m_Front));
		UpdateViewMatrix();
		m_ViewProjection = m_ProjectionMatrix * m_ViewMatrix;
	}

	std::shared_ptr<CameraController> CameraController::Create(ControllerType type, std::shared_ptr<Camera> camera)
	{
		switch (type)
		{
		case Engine::CameraController::ControllerType::FPS:
			return std::shared_ptr<FPSCameraController>(New<FPSCameraController>(camera), Delete<FPSCameraController>);
		default:
			break;
		}
	}

	const F32 FPSCameraController::DEFAULT_TRANSLATION_SPEED = 1.0f;
	const F32 FPSCameraController::DEFAULT_ROTATION_SPEED = 10.0f;
	const F32 FPSCameraController::DEFAULT_E_YAW = -90.0f;
	const F32 FPSCameraController::DEFAULT_E_PITCH = 0.0f;
	const F32 FPSCameraController::DEFAULT_E_ROLL = 0.0f;

	FPSCameraController::FPSCameraController(std::shared_ptr<Camera> camera) : m_Camera(camera),
		m_TranslationSpeed(DEFAULT_TRANSLATION_SPEED), m_RotationSpeed(DEFAULT_ROTATION_SPEED),
		m_Yaw(DEFAULT_E_YAW), m_Pitch(DEFAULT_E_PITCH), m_Roll(DEFAULT_E_ROLL)
	{ 
		m_MouseCoords = Input::MousePosition();
		m_Camera->SetFrontDirection(GetNewFrontVector(0.0f, 0.0f));
	}

	void FPSCameraController::OnUpdate(F32 dt)
	{
		if (m_MouseMoved)
		{
			F32 prevMouseX = m_MouseCoords.x;
			F32 prevMouseY = m_MouseCoords.y;
			m_MouseCoords = Input::MousePosition();

			F32 xOffset = m_MouseCoords.x - prevMouseX;
			F32 yOffset = m_MouseCoords.y - prevMouseY;

			xOffset *= m_RotationSpeed * dt; yOffset *= -m_RotationSpeed * dt;

			glm::vec3 newFrontDirection = GetNewFrontVector(xOffset, yOffset);
			m_Camera->SetFrontDirection(newFrontDirection);
			m_MouseMoved = false;
		}		
		if (Input::GetKey(Key::W))
		{
			m_Camera->SetPosition(m_Camera->GetPostion() + m_TranslationSpeed * dt * m_Camera->GetFrontDirection());
		}
		if (Input::GetKey(Key::S))
		{
			m_Camera->SetPosition(m_Camera->GetPostion() - m_TranslationSpeed * dt * m_Camera->GetFrontDirection());
		}
		if (Input::GetKey(Key::A))
		{
			m_Camera->SetPosition(m_Camera->GetPostion() - m_TranslationSpeed * dt * m_Camera->GetRightDirection());
		}
		if (Input::GetKey(Key::D))
		{
			m_Camera->SetPosition(m_Camera->GetPostion() + m_TranslationSpeed * dt * m_Camera->GetRightDirection());
		}


	}

	bool FPSCameraController::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseMovedEvent>([this](Event& e) { this->SetMouseMoved(true); return false; });
		return false;
	}

	glm::vec3 FPSCameraController::GetNewFrontVector(F32 xOffset, F32 yOffset)
	{
		m_Yaw += xOffset;
		m_Pitch += yOffset;

		if (m_Pitch > 89.99f) m_Pitch = 89.99f;
		else if (m_Pitch < -89.99f) m_Pitch = -89.99f;

		glm::vec3 newFrontDirection;
		newFrontDirection.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		newFrontDirection.y = sin(glm::radians(m_Pitch));
		newFrontDirection.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		newFrontDirection = glm::normalize(newFrontDirection);
		return newFrontDirection;
	}

}