#pragma once
#include "Camera.h"
#include "HRealEngine/Core/Timestep.h"
#include "HRealEngine/Events/EventBase.h"
#include "HRealEngine/Events/MouseEvent.h"

namespace HRealEngine
{
    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);
        
        void OnUpdate(Timestep ts);
        void OnEvent(EventBase& e);

        inline float GetDistance() const { return m_Distance; }
        inline void SetDistance(float distance) { m_Distance = distance; }

        inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

        glm::vec3 GetUpDirection() const;
        glm::vec3 GetRightDirection() const;
        glm::vec3 GetForwardDirection() const;
        const glm::vec3& GetPosition() const { return m_Position; }
        glm::quat GetOrientation() const;

        float GetPitch() const { return m_Pitch; }
        float GetYaw() const { return m_Yaw; }
    private:
        void UpdateProjection();
        void UpdateView();
        
        bool OnMouseScroll(MouseScrolledEvent& e);

        void MousePan(const glm::vec2& delta);
        void MouseRotate(const glm::vec2& delta);
        void MouseZoom(float delta);

        glm::vec3 CalculatePosition() const;
        std::pair<float, float> PanSpeed() const;
        float RotationSpeed() const;
        float ZoomSpeed() const;
        
        float m_FOV = 45.f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.f;
        
        glm::mat4 m_ViewMatrix;
        glm::vec3 m_Position = {0.f, 0.f, -10.f};
        glm::vec3 m_FocalPoint = {0.f, 0.f, 0.f};
        
        glm::vec2 m_InitialMousePosition = {0.f, 0.f};
        
        float m_Distance = 10.f;
        float m_Pitch = 0.f, m_Yaw = 0.f;
        float m_ViewportWidth = 1280.f, m_ViewportHeight = 720.f;
    };
}
