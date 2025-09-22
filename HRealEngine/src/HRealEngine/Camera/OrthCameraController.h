

#pragma once
#include "OrthCamera.h"
#include "HRealEngine/Core/Timestep.h"
#include "HRealEngine/Events/AppEvent.h"
#include "HRealEngine/Events/MouseEvent.h"

namespace HRealEngine
{
    struct OrthCameraBounds
    {
        float Left, Right;
        float Bottom, Top;

        float GetWidth() const { return Right - Left; }
        float GetHeight() const { return Top - Bottom; }
    };
    class OrthCameraController
    {
    public:
        OrthCameraController(float aspectRatio, bool rotation = false);

        void OnUpdate(Timestep timestep);
        void OnEvent(EventBase& eventRef);

        void OnResize(float width, float height);

        OrthCamera& GetCamera() { return m_OrthCamera; }
        const OrthCamera& GetCamera() const { return m_OrthCamera; }

        float GetZoomLevel() const { return m_ZoomLevel; }
        void SetZoomLevel(float level) { m_ZoomLevel = level; CalculateView(); }

        const OrthCameraBounds& GetBounds() const { return m_CameraBounds; }
    private:
        void CalculateView();
        bool OnMouseScrolled(MouseScrolledEvent& eventRef);
        bool OnWindowResized(WindowResizeEvent& eventRef);
        
        float m_AspectRatio;
        float m_ZoomLevel = 1.0f;
        bool m_bRotationRef;
        OrthCameraBounds m_CameraBounds;
        OrthCamera m_OrthCamera;

        glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
        float m_CameraMoveSpeed = 2.f, m_CameraRotationSpeed = 90.0f;
        float m_CameraRotation = 0.0f;
    };
}
