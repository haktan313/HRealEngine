

#pragma once
#include "glm/glm.hpp"

namespace HRealEngine
{
    class OrthCamera
    {
    public:
        OrthCamera(float left, float right, float bottom, float top);

        const glm::vec3& GetPosition() const { return m_Position; }
        void SetPosition(const glm::vec3& newPosition) {m_Position = newPosition; RecalculateViewProjectionMatrix(); }
        
        float GetRotation() const { return m_Rotation; }
        void SetRotation(const float newRotation) {m_Rotation = newRotation; RecalculateViewProjectionMatrix(); }

        void SetProjection(float left, float right, float bottom, float top);

        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; } 
    private:
        void RecalculateViewProjectionMatrix();
        
        glm::mat4 m_ProjectionMatrix;//Decides how 3D world space is projected into 2D screen space.no perspective, useful for 2D or UI
        glm::mat4 m_ViewMatrix;//Represents the camera's transform (position + rotation)
        glm::mat4 m_ViewProjectionMatrix;

        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        float m_Rotation = 0.0f;
    };
}
