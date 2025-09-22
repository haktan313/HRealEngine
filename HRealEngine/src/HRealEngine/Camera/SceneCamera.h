

#pragma once
#include "Camera.h"

namespace HRealEngine
{
    class SceneCamera : public Camera
    {
    public:
        enum class ProjectionType
        {
            Perspective = 0, Orthographic = 1
        };
        SceneCamera();
        virtual ~SceneCamera() = default;

        void SetOrthographic(float size, float nearClip, float farClip);
        void SetPerspective(float verticalFOV, float nearClip, float farClip);
        void SetViewportSize(uint32_t width, uint32_t height);
        void RecalculateProjection();

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

        float GetOrthographicSize() const { return m_OrthSize; }
        float GetNearClip() const { return m_OrthNear; }
        float GetFarClip() const { return m_OrthFar; }

        float GetPerspectiveFOV() const { return m_PerspectiveFOV; }
        float GetPerspectiveNear() const { return m_PerspectiveNear; }
        float GetPerspectiveFar() const { return m_PerspectiveFar; }
        
        void SetOrthographicSize(float size) { m_OrthSize = size; RecalculateProjection(); }
        void SetNearClip(float nearClip) { m_OrthNear = nearClip; RecalculateProjection(); }
        void SetFarClip(float farClip) { m_OrthFar = farClip; RecalculateProjection(); }

        void SetPerspectiveFOV(float fov) { m_PerspectiveFOV = fov; RecalculateProjection(); }
        void SetPerspectiveNear(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
        void SetPerspectiveFar(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }
    private:
        ProjectionType m_ProjectionType = ProjectionType::Orthographic;
        
        float m_OrthSize = 10.0f;
        float m_OrthNear = -1.0f, m_OrthFar = 1.0f;

        float m_PerspectiveFOV = glm::radians(45.0f);
        float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

        float m_AspectRatio = 0.0f;
    };
}
