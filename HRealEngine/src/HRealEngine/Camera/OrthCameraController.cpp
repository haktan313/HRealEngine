


#include "HRpch.h"
#include "OrthCameraController.h"
#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Core/KeyCodes.h"

namespace HRealEngine
{
    OrthCameraController::OrthCameraController(float aspectRatio, bool rotation)
        : m_AspectRatio(aspectRatio), m_bRotationRef(rotation), m_CameraBounds({ -aspectRatio * m_ZoomLevel, aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel }),
            m_OrthCamera(m_CameraBounds.Left, m_CameraBounds.Right, m_CameraBounds.Bottom, m_CameraBounds.Top)
    {
    }

    void OrthCameraController::OnUpdate(Timestep timestep)
    {
        if (Input::IsKeyPressed(HR_KEY_A))
            m_CameraPosition.x -= m_CameraMoveSpeed * timestep;
        else if (Input::IsKeyPressed(HR_KEY_D))
            m_CameraPosition.x += m_CameraMoveSpeed * timestep;
        if (Input::IsKeyPressed(HR_KEY_W))
            m_CameraPosition.y += m_CameraMoveSpeed * timestep;
        else if (Input::IsKeyPressed(HR_KEY_S))
            m_CameraPosition.y -= m_CameraMoveSpeed * timestep;

        if (m_bRotationRef)
        {
            if (Input::IsKeyPressed(HR_KEY_Q))
                m_CameraRotation -= m_CameraRotationSpeed * timestep;
            else if (Input::IsKeyPressed(HR_KEY_E))
                m_CameraRotation += m_CameraRotationSpeed * timestep;
            m_OrthCamera.SetRotation(m_CameraRotation);
        }
        m_OrthCamera.SetPosition(m_CameraPosition);
        m_CameraMoveSpeed = m_ZoomLevel;
    }

    void OrthCameraController::OnEvent(EventBase& eventRef)
    {
        EventDispatcher dispatcher(eventRef);
        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthCameraController::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrthCameraController::OnWindowResized));
    }

    void OrthCameraController::OnResize(float width, float height)
    {
        m_AspectRatio = width / height;
        CalculateView();
    }

    void OrthCameraController::CalculateView()
    {
        m_CameraBounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
        m_OrthCamera.SetProjection(m_CameraBounds.Left, m_CameraBounds.Right, m_CameraBounds.Bottom, m_CameraBounds.Top);
    }

    bool OrthCameraController::OnMouseScrolled(MouseScrolledEvent& eventRef)
    {
        m_ZoomLevel -= eventRef.GetOffsetY() * 0.25f;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        CalculateView();
        return false;
    }

    bool OrthCameraController::OnWindowResized(WindowResizeEvent& eventRef)
    {
        OnResize((float)eventRef.GetWidth(), (float)eventRef.GetHeight());
        return false;
    }
}
