

#include "HRpch.h"
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
        m_OrthSize = size;
        m_OrthNear = nearClip;
        m_OrthFar = farClip;
        RecalculateProjection();
    }

    void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Perspective;
        m_PerspectiveFOV = verticalFOV;
        m_PerspectiveNear = nearClip;
        m_PerspectiveFar = farClip;
        RecalculateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
        if (height == 0)
            m_AspectRatio = 0.0f;
        else
            m_AspectRatio = (float)width / (float)height;
        RecalculateProjection();
    }

    void SceneCamera::RecalculateProjection()
    {
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            m_ProjectionMatrix = glm::perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
        }
        else
        {
            float left = -m_OrthSize * m_AspectRatio * 0.5f;
            float right = m_OrthSize * m_AspectRatio * 0.5f;
            float bottom = -m_OrthSize * 0.5f;
            float top = m_OrthSize * 0.5f;
    
            m_ProjectionMatrix = glm::ortho(left, right, bottom, top, m_OrthNear, m_OrthFar);
        }
    }

}
