#include "enginepch.h"

#include "Camera.h"

#include "Engine/Core/Input.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Engine
{
	const glm::vec3 Camera::DEFAULT_POSITION						= glm::vec3(0.0f);
	const glm::quat Camera::DEFAULT_ORIENTATION						= glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	const F32  Camera::DEFAULT_FOV									= 45.0f;
	const F32  Camera::DEFAULT_ASPECT								= 16.0f / 9.0f;
	const F32  Camera::DEFAULT_NEAR									= 0.03f;
	const F32  Camera::DEFAULT_FAR									= 1000.0f;
	const Camera::ProjectionType Camera::DEFAULT_PROJECTION_TYPE	= Camera::ProjectionType::Perspective;


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
		m_Position(DEFAULT_POSITION), m_Orientation(DEFAULT_ORIENTATION)
	{
		UpdateViewMatrix();
		UpdateProjectionMatrix();
		UpdateViewProjection();
	}

	Camera::Camera(const glm::vec3& position, F32 fov, F32 aspect) : m_FieldOfView(fov), m_Aspect(aspect),
		m_NearClipPlane(DEFAULT_NEAR), m_FarClipPlane(DEFAULT_FAR), m_ProjectionType(DEFAULT_PROJECTION_TYPE),
		m_Position(position), m_Orientation(DEFAULT_ORIENTATION)
	{
		UpdateViewMatrix();
		UpdateProjectionMatrix();
		UpdateViewProjection();
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
	}

	void Camera::SetOrientation(const glm::quat& orientation)
	{
		m_Orientation = orientation;
	}

	void Camera::SetProjection(ProjectionType type)
	{
		m_ProjectionType = type;
		UpdateProjectionMatrix();
		UpdateViewProjection();
	}

	glm::vec3 Camera::GetForward() const
	{
		return glm::rotate(m_Orientation, glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 Camera::GetUp() const
	{
		return glm::rotate(m_Orientation, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Camera::GetRight() const
	{
		return glm::rotate(m_Orientation, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	void Camera::UpdateViewMatrix()
	{
		m_ViewMatrix = glm::toMat4(glm::inverse(m_Orientation)) * glm::translate(glm::mat4(1.0f), -m_Position);
	}

	void Camera::UpdateProjectionMatrix()
	{
		switch (m_ProjectionType)
		{
		case ProjectionType::Perspective:
			m_ProjectionMatrix = glm::perspective(m_FieldOfView, m_Aspect, m_NearClipPlane, m_FarClipPlane);
			break;
		case ProjectionType::Orthographic:
			m_ProjectionMatrix = glm::ortho(
				-1.0f * m_Aspect * m_OrthoZoom, 1.0f * m_Aspect * m_OrthoZoom,
				-1.0f * m_OrthoZoom, 1.0f * m_OrthoZoom,
				m_NearClipPlane, m_FarClipPlane
			);
			break;
		}
	}

	void Camera::UpdateViewProjection()
	{
		m_ViewProjection = m_ProjectionMatrix * m_ViewMatrix;
	}

	std::shared_ptr<CameraController> CameraController::Create(ControllerType type, std::shared_ptr<Camera> camera)
	{
		switch (type)
		{
		case Engine::CameraController::ControllerType::FPS:
			return std::shared_ptr<FPSCameraController>(New<FPSCameraController>(camera), Delete<FPSCameraController>);
		case Engine::CameraController::ControllerType::Editor:
			return std::shared_ptr<EditorCameraController>(New<EditorCameraController>(camera), Delete<EditorCameraController>);
		default:
			return std::shared_ptr<FPSCameraController>(New<FPSCameraController>(camera), Delete<FPSCameraController>);
		}
	}

	const F32 FPSCameraController::DEFAULT_TRANSLATION_SPEED		= 1.0f;
	const F32 FPSCameraController::DEFAULT_SENSITIVITY				= 0.005f;
	const F32 FPSCameraController::DEFAULT_E_YAW					= 0.0f;
	const F32 FPSCameraController::DEFAULT_E_PITCH					= 0.0f;

	FPSCameraController::FPSCameraController(std::shared_ptr<Camera> camera) : m_Camera(camera),
		m_TranslationSpeed(DEFAULT_TRANSLATION_SPEED), m_MouseSensitivity(DEFAULT_SENSITIVITY),
		m_Yaw(DEFAULT_E_YAW), m_Pitch(DEFAULT_E_PITCH)
	{ 
		m_MouseCoords = Input::MousePosition();
		m_Camera->UpdateViewMatrix();
		m_Camera->UpdateViewProjection();
	}

	void FPSCameraController::OnUpdate(F32 dt)
	{		
		if (Input::GetKey(Key::W))
		{
			m_Camera->SetPosition(m_Camera->GetPosition() + m_TranslationSpeed * dt * m_Camera->GetForward());
		}
		if (Input::GetKey(Key::S))
		{
			m_Camera->SetPosition(m_Camera->GetPosition() - m_TranslationSpeed * dt * m_Camera->GetForward());
		}
		if (Input::GetKey(Key::A))
		{
			m_Camera->SetPosition(m_Camera->GetPosition() - m_TranslationSpeed * dt * m_Camera->GetRight());
		}
		if (Input::GetKey(Key::D))
		{
			m_Camera->SetPosition(m_Camera->GetPosition() + m_TranslationSpeed * dt * m_Camera->GetRight());
		}
		m_Camera->UpdateViewMatrix();
		m_Camera->UpdateViewProjection();
	}

	bool FPSCameraController::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) {
			F32 prevMouseX = m_MouseCoords.x;
			F32 prevMouseY = m_MouseCoords.y;

			m_MouseCoords.x = e.GetX();
			m_MouseCoords.y = e.GetY();

			F32 xOffset = m_MouseCoords.x - prevMouseX;
			F32 yOffset = m_MouseCoords.y - prevMouseY;

			xOffset *= m_MouseSensitivity; yOffset *= m_MouseSensitivity;

			m_Yaw += xOffset;
			m_Pitch += yOffset;

			if (m_Pitch > glm::radians(89.99f)) m_Pitch = glm::radians(89.99f);
			else if (m_Pitch < glm::radians(-89.99f)) m_Pitch = glm::radians(-89.99f);

			glm::quat newOrientation = glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
			m_Camera->SetOrientation(newOrientation);
			m_Camera->UpdateViewMatrix();

			return false;
		});
		return false;
	}

	const F32 EditorCameraController::DEFAULT_TRANSLATION_SPEED			= 0.1f;
	const F32 EditorCameraController::DEFAULT_ROTATION_SPEED			= 0.5f;
	const F32 EditorCameraController::DEFAULT_E_YAW						= 0.0f;
	const F32 EditorCameraController::DEFAULT_E_PITCH					= 0.0f;
	const F32 EditorCameraController::DEFAULT_DISTANCE					= 1.0f;
	const glm::vec3 EditorCameraController::DEFAULT_FOCAL_POINT			= glm::vec3(0.0);

	EditorCameraController::EditorCameraController(std::shared_ptr<Camera> camera) : m_Camera(camera),
		m_TranslationSpeed(DEFAULT_TRANSLATION_SPEED), m_RotationSpeed(DEFAULT_ROTATION_SPEED),
		m_Yaw(DEFAULT_E_YAW), m_Pitch(DEFAULT_E_PITCH), 
		m_Distance(DEFAULT_DISTANCE), m_FocalPoint(DEFAULT_FOCAL_POINT)
	{
		m_MouseCoords = Input::MousePosition();
		m_Camera->SetPosition(m_FocalPoint - m_Distance * m_Camera->GetForward());
		m_Camera->UpdateViewMatrix();
	}

	void EditorCameraController::OnUpdate(F32 dt)
	{
		if (Input::GetKey(Key::LeftAlt))
		{
			F32 prevMouseX = m_MouseCoords.x;
			F32 prevMouseY = m_MouseCoords.y;
			m_MouseCoords = Input::MousePosition();
			F32 xOffset = m_MouseCoords.x - prevMouseX;
			F32 yOffset = m_MouseCoords.y - prevMouseY;


			if (Input::GetKey(Key::LeftShift))
			{
				if (Input::GetMouseButton(Mouse::Button0))
				{
					xOffset *= -m_TranslationSpeed * dt * m_Distance, yOffset *= m_TranslationSpeed * dt * m_Distance;
					m_FocalPoint += glm::vec3(xOffset, yOffset, 0.0f);
					glm::vec3 newCameraPosition = m_FocalPoint - m_Distance * m_Camera->GetForward();
					m_Camera->SetPosition(newCameraPosition);
					m_Camera->UpdateViewMatrix();
					m_Camera->UpdateViewProjection();
				}
			}
			else
			{
				if (Input::GetMouseButton(Mouse::Button0))
				{
					xOffset *= m_RotationSpeed * dt, yOffset *= -m_RotationSpeed * dt;
					F32 yawSign = m_Camera->GetUp().y < 0 ? 1.0f : -1.0f;
					m_Yaw += xOffset * yawSign * m_RotationSpeed;
					m_Pitch += yOffset * m_RotationSpeed;

					glm::quat newOrientation = glm::quat(glm::vec3(m_Pitch, m_Yaw, 0.0f));
					m_Camera->SetOrientation(newOrientation);
					glm::vec3 newPosition = m_FocalPoint - m_Distance * m_Camera->GetForward();
					m_Camera->SetPosition(newPosition);

					m_Camera->UpdateViewMatrix();
					m_Camera->UpdateViewProjection();
				}

				if (Input::GetMouseButton(Mouse::Button1))
				{
					m_Distance += yOffset * ZoomSpeed() * dt;
					glm::vec3 newCameraPosition = m_FocalPoint - m_Distance * m_Camera->GetForward();
					m_Camera->m_OrthoZoom = m_Distance;
					m_Camera->SetPosition(newCameraPosition);
					m_Camera->UpdateViewMatrix();
					if (m_Camera->m_ProjectionType == Camera::ProjectionType::Orthographic) m_Camera->UpdateProjectionMatrix();
					m_Camera->UpdateViewProjection();
				}
			}				
		}	
	}

	bool EditorCameraController::OnEvent(Event& event)
	{
		return false;
	}

	F32 EditorCameraController::ZoomSpeed()
	{
		F32 dist = std::max(m_Distance * 0.2f, 0.0f);
		F32 speed = dist * dist;
		speed = std::min(speed, 100.0f);
		return speed;
	}
}