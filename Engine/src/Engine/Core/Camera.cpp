#include "enginepch.h"

#include "Camera.h"

#include "Engine/Core/Input.h"
#include "Engine/Memory/MemoryManager.h"

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


	Ref<Camera> Camera::Create()
	{
		return CreateRef<Camera>();
	}

	Ref<Camera> Camera::Create(const glm::vec3& position, F32 fov, F32 aspect)
	{
		return CreateRef<Camera>(position, fov, aspect);
	}

	Camera::Camera()
		: 	m_ProjectionType(DEFAULT_PROJECTION_TYPE), m_Aspect(DEFAULT_ASPECT),
			m_NearClipPlane(DEFAULT_NEAR), m_FarClipPlane(DEFAULT_FAR), m_FieldOfView(DEFAULT_FOV),
			m_Position(DEFAULT_POSITION), m_Orientation(DEFAULT_ORIENTATION),
			m_ViewProjection(glm::mat4{1.0f}), m_ViewProjectionInverse(glm::mat4{1.0f}),
			m_ViewMatrix(glm::mat4{1.0f}), m_ProjectionMatrix(glm::mat4{1.0f})
	{
		UpdateViewMatrix();
	}

	Camera::Camera(const glm::vec3& position, F32 fov, F32 aspect)
	:	m_ProjectionType(DEFAULT_PROJECTION_TYPE), m_Aspect(aspect),
		m_NearClipPlane(DEFAULT_NEAR), m_FarClipPlane(DEFAULT_FAR),
		m_FieldOfView(fov),
		m_Position(position), m_Orientation(DEFAULT_ORIENTATION),
		m_ViewProjection(glm::mat4{1.0f}), m_ViewProjectionInverse(glm::mat4{1.0f}),
		m_ViewMatrix(glm::mat4{1.0f}), m_ProjectionMatrix(glm::mat4{1.0f})
	{
		UpdateViewMatrix();
	}

	void Camera::SetViewport(U32 width, U32 height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
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

	void Camera::SetZoom(F32 zoom)
	{
		m_OrthoZoom = zoom;
		UpdateProjectionMatrix();
		UpdateViewProjection();
	}

	void Camera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_FN(Camera::OnWindowResize));
	}

	F32 Camera::GetPixelCoefficient()
	{
		if (m_ProjectionType == ProjectionType::Perspective) return 1.0f;
		return GetPixelCoefficient(m_OrthoZoom);
	}

	F32 Camera::GetPixelCoefficient(F32 distance) const
	{
		if (m_ProjectionType == ProjectionType::Perspective) return 1.0f;
		return 2.0f * distance / (F32)m_ViewportHeight;
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
		m_Aspect = F32(m_ViewportWidth) / F32(m_ViewportHeight);
		switch (m_ProjectionType)
		{
		case ProjectionType::Perspective:
			m_ProjectionMatrix = glm::perspective(m_FieldOfView, m_Aspect, m_NearClipPlane, m_FarClipPlane);
			break;
		case ProjectionType::Orthographic:
			m_ProjectionMatrix = glm::ortho(
				-1.0f * m_Aspect * m_OrthoZoom, 1.0f * m_Aspect * m_OrthoZoom,
				-1.0f * m_OrthoZoom, 1.0f * m_OrthoZoom,
				-1.0f, m_FarClipPlane
			);
			break;
		}
	}

	void Camera::UpdateViewProjection()
	{
		m_ViewProjection = m_ProjectionMatrix * m_ViewMatrix;
		m_ViewProjectionInverse = glm::inverse(m_ViewProjection);
	}

	bool Camera::OnWindowResize(WindowResizeEvent& event)
	{
		SetViewport(event.GetWidth(), event.GetHeight());
		return false;
	}

	glm::vec2 Camera::ScreenToWorldPoint(const glm::vec2& screenPosition) const
	{
		ENGINE_CORE_ASSERT(m_ProjectionType == ProjectionType::Orthographic, "Use raycast instead")
		glm::vec2 modified;
		modified.x = screenPosition.x * 2.0f / F32(m_ViewportWidth) - 1;
		modified.y = screenPosition.y * 2.0f / F32(m_ViewportHeight) - 1;

		glm::vec4 augmented = glm::vec4(modified, 0, 1.0f);
		glm::vec4 worldCoords = m_ViewProjectionInverse * augmented;

		return glm::vec2(worldCoords);
	}

	glm::vec2 Camera::WorldToScreenPoint(const glm::vec2& worldPosition) const
	{
		return glm::vec2(m_ViewMatrix * glm::vec4(worldPosition, 0, 1.0f));
	}

	CameraController::CameraController(Ref<Camera> camera)
		: m_Camera(camera)
	{
	}

	Ref<CameraController> CameraController::Create(ControllerType type, Ref<Camera> camera)
	{
		Ref<CameraController> newCam;
		switch (type)
		{
		case Engine::CameraController::ControllerType::FPS: newCam = CreateRef<FPSCameraController>(camera); break;
		case Engine::CameraController::ControllerType::Editor: newCam = CreateRef<EditorCameraController>(camera); break;
		case Engine::CameraController::ControllerType::Editor2D: newCam = CreateRef<Editor2DCameraController>(camera); break;
		}
		newCam->SetControllerType(type);
		return newCam;
	}

	const F32 FPSCameraController::DEFAULT_TRANSLATION_SPEED		= 1.0f;
	const F32 FPSCameraController::DEFAULT_SENSITIVITY				= 0.005f;
	const F32 FPSCameraController::DEFAULT_E_YAW					= 0.0f;
	const F32 FPSCameraController::DEFAULT_E_PITCH					= 0.0f;

	FPSCameraController::FPSCameraController(Ref<Camera> camera)
		: CameraController(camera),
		m_TranslationSpeed(DEFAULT_TRANSLATION_SPEED), m_MouseSensitivity(DEFAULT_SENSITIVITY),
		m_Yaw(DEFAULT_E_YAW), m_Pitch(DEFAULT_E_PITCH), m_MouseCoords(glm::vec2{0.0f})
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
			F32 yOffset = -(m_MouseCoords.y - prevMouseY);

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
	const F32 EditorCameraController::DEFAULT_DISTANCE					= 5.0f;
	const glm::vec3 EditorCameraController::DEFAULT_FOCAL_POINT			= glm::vec3(0.0);

	EditorCameraController::EditorCameraController(Ref<Camera> camera)
		: CameraController(camera),
		m_TranslationSpeed(DEFAULT_TRANSLATION_SPEED), m_RotationSpeed(DEFAULT_ROTATION_SPEED),
		m_Yaw(DEFAULT_E_YAW), m_Pitch(DEFAULT_E_PITCH),
		m_MouseCoords(0, 0), m_FocalPoint(DEFAULT_FOCAL_POINT),
		m_Distance(DEFAULT_DISTANCE)
	{
		m_MouseCoords = Input::MousePosition();
		m_Distance = m_Camera->m_OrthoZoom;
		m_Camera->SetPosition(m_FocalPoint - m_Distance * m_Camera->GetForward());
		m_Camera->UpdateViewMatrix();
	}

	void EditorCameraController::OnUpdate(F32 dt)
	{
		if (Input::GetKey(Key::LeftAlt))
		{
			const F32 prevMouseX = m_MouseCoords.x;
			const F32 prevMouseY = m_MouseCoords.y;
			m_MouseCoords = Input::MousePosition();
			F32 xOffset = m_MouseCoords.x - prevMouseX;
			F32 yOffset = -(m_MouseCoords.y - prevMouseY);


			if (Input::GetKey(Key::LeftShift))
			{
				if (Input::GetMouseButton(Mouse::Button0))
				{
					xOffset *= -m_TranslationSpeed * dt * m_Distance; yOffset *= m_TranslationSpeed * dt * m_Distance;
					m_FocalPoint += glm::vec3(xOffset, yOffset, 0.0f);
					const glm::vec3 newCameraPosition = m_FocalPoint - m_Distance * m_Camera->GetForward();
					m_Camera->SetPosition(newCameraPosition);
					m_Camera->UpdateViewMatrix();
					m_Camera->UpdateViewProjection();
				}
			}
			else
			{
				if (!m_AnglesConstrained && Input::GetMouseButton(Mouse::Button0))
				{
					xOffset *= m_RotationSpeed * dt; yOffset *= -m_RotationSpeed * dt;
					const F32 yawSign = m_Camera->GetUp().y < 0 ? 1.0f : -1.0f;
					m_Yaw += xOffset * yawSign * m_RotationSpeed;
					m_Pitch += yOffset * m_RotationSpeed;

					const glm::quat newOrientation = glm::quat(glm::vec3(m_Pitch, m_Yaw, 0.0f));
					m_Camera->SetOrientation(newOrientation);
					const glm::vec3 newPosition = m_FocalPoint - m_Distance * m_Camera->GetForward();
					m_Camera->SetPosition(newPosition);

					m_Camera->UpdateViewMatrix();
					m_Camera->UpdateViewProjection();
				}

				if (Input::GetMouseButton(Mouse::Button1))
				{
					const F32 deltaDistance = yOffset * ZoomSpeed() * dt;
					m_Distance += deltaDistance;
					if (m_Distance < 0.0f) m_Distance = 1;
					const glm::vec3 newCameraPosition = m_FocalPoint - m_Distance * m_Camera->GetForward();
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

	F32 EditorCameraController::ZoomSpeed() const
	{
		const F32 dist = std::max(m_Distance * 0.2f, 0.0f);
		F32 speed = dist * dist;
		speed = std::min(speed, 100.0f);
		return speed;
	}
}