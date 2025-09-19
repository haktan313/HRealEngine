

//OrthCamera.h
#pragma once
#include "glm/glm.hpp"

namespace HRealEngine
{
    class OrthCamera
    {
    public:
        OrthCamera(float left, float right, float bottom, float top);

        const glm::vec3& GetPosition() const { return position; }
        void SetPosition(const glm::vec3& newPosition) {position = newPosition; RecalculateViewProjectionMatrix(); }
        
        float GetRotation() const { return rotation; }
        void SetRotation(const float newRotation) {rotation = newRotation; RecalculateViewProjectionMatrix(); }

        void SetProjection(float left, float right, float bottom, float top);

        const glm::mat4& GetProjectionMatrix() const { return projectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return viewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return viewProjectionMatrix; } 
    private:
        void RecalculateViewProjectionMatrix();
        
        glm::mat4 projectionMatrix;//Decides how 3D world space is projected into 2D screen space.no perspective, useful for 2D or UI
        glm::mat4 viewMatrix;//Represents the camera�s transform (position + rotation)
        glm::mat4 viewProjectionMatrix;

        glm::vec3 position = { 0.0f, 0.0f, 0.0f };
        float rotation = 0.0f;
    };
}
