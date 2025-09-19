
//SceneCamera.h
#pragma once
#include "HRealEngine/Renderer/Camera.h"

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

        float GetOrthographicSize() const { return OrthSize; }
        float GetNearClip() const { return OrthNear; }
        float GetFarClip() const { return OrthFar; }

        float GetPerspectiveFOV() const { return PerspectiveFOV; }
        float GetPerspectiveNear() const { return PerspectiveNear; }
        float GetPerspectiveFar() const { return PerspectiveFar; }
        
        void SetOrthographicSize(float size) { OrthSize = size; RecalculateProjection(); }
        void SetNearClip(float nearClip) { OrthNear = nearClip; RecalculateProjection(); }
        void SetFarClip(float farClip) { OrthFar = farClip; RecalculateProjection(); }

        void SetPerspectiveFOV(float fov) { PerspectiveFOV = fov; RecalculateProjection(); }
        void SetPerspectiveNear(float nearClip) { PerspectiveNear = nearClip; RecalculateProjection(); }
        void SetPerspectiveFar(float farClip) { PerspectiveFar = farClip; RecalculateProjection(); }
    private:
        ProjectionType m_ProjectionType = ProjectionType::Orthographic;
        
        float OrthSize = 10.0f;
        float OrthNear = -1.0f, OrthFar = 1.0f;

        float PerspectiveFOV = glm::radians(45.0f);
        float PerspectiveNear = 0.01f, PerspectiveFar = 1000.0f;

        float AspectRatio = 0.0f;
    };
}
