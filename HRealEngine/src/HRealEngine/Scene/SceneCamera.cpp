
//SceneCamera.cpp
#include "SceneCamera.h"

#include "glm/ext/matrix_clip_space.hpp"

namespace HRealEngine
{
    SceneCamera::SceneCamera()
    {
        RecalculateProjection();
    }

    void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        OrthSize = size;
        OrthNear = nearClip;
        OrthFar = farClip;
        RecalculateProjection();
    }

    void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Perspective;
        PerspectiveFOV = verticalFOV;
        PerspectiveNear = nearClip;
        PerspectiveFar = farClip;
        RecalculateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
        if (height == 0)
            AspectRatio = 0.0f;
        else
            AspectRatio = (float)width / (float)height;
        RecalculateProjection();
    }

    void SceneCamera::RecalculateProjection()
    {
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            m_ProjectionMatrix = glm::perspective(PerspectiveFOV, AspectRatio, PerspectiveNear, PerspectiveFar);
        }
        else
        {
            float left = -OrthSize * AspectRatio * 0.5f;
            float right = OrthSize * AspectRatio * 0.5f;
            float bottom = -OrthSize * 0.5f;
            float top = OrthSize * 0.5f;
    
            m_ProjectionMatrix = glm::ortho(left, right, bottom, top, OrthNear, OrthFar);
        }
    }

}
