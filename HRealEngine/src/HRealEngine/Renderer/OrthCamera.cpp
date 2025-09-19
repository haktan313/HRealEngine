
//OrthCamera.cpp
#include "OrthCamera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace HRealEngine
{
    OrthCamera::OrthCamera(float left, float right, float bottom, float top) : projectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), viewMatrix(1.f)
    {
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }

    void OrthCamera::SetProjection(float left, float right, float bottom, float top)
    {
        projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }

    void OrthCamera::RecalculateViewProjectionMatrix()
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        viewMatrix = glm::inverse(transform);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }
}
